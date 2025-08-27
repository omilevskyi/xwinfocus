#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xwinfocus.h"

#define PROG "xwinfocus"

#define NET_ACTIVE_WINDOW "_NET_ACTIVE_WINDOW"
#define NET_CLIENT_LIST "_NET_CLIENT_LIST"
#define X11_ATOM_NAME "_XWINFOCUS_PREVIOUS_WINDOW"

// (int)(sizeof(unsigned long) * 2)
#define WINID_FMT_LEN (int)(sizeof(unsigned long))

typedef struct {
  int store_previous; /* store current active window id into a property */
  int verbose;
  int wait_ms;
} options_t;

static options_t options = {1, 0, 0};

static void die(int rc, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: ", PROG);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(rc);
}

static void warn(const char *fmt, ...) {
  if (options.verbose) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "%s: ", PROG);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
  }
}

static void nsleep_ms(int ms) {
  if (ms > 0) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
      warn("nanosleep() failed"); /* retry */
    }
  }
}

static const char *fringe(const char *str, char *left, char *right) {
  if (str) {
    *left = *right = '"';
    return str;
  }
  *left = '<';
  *right = '>';
  return "null";
}

static Window find_window(Display *dpy, Window root, const char *target_name,
                          const char *target_class) {
  Atom net_client_list = XInternAtom(dpy, NET_CLIENT_LIST, True);
  if (net_client_list == None)
    return 0;

  Atom ret_type;
  int ret_format;
  unsigned long num_items, bytes_after;
  unsigned char *data = NULL;
  Status status = XGetWindowProperty(dpy, root, net_client_list, 0, (~0L),
                                     False, XA_WINDOW, &ret_type, &ret_format,
                                     &num_items, &bytes_after, &data);
  if (status != Success || ret_type == None || data == NULL) {
    if (data)
      XFree(data);
    return 0;
  }

  Window *windows = (Window *)data;
  for (unsigned long i = 0; i < num_items; i++) {
    XClassHint hint;
    hint.res_name = hint.res_class = NULL;
    if (XGetClassHint(dpy, windows[i], &hint)) {
      if (options.verbose) {
        char name_left, name_right, class_left, class_right;
        const char *name_body = fringe(hint.res_name, &name_left, &name_right);
        const char *class_body =
            fringe(hint.res_class, &class_left, &class_right);
        fprintf(stderr, "id: 0x%0*lx\tname: %c%s%c\tclass: %c%s%c\n",
                WINID_FMT_LEN, (unsigned long)windows[i], name_left, name_body,
                name_right, class_left, class_body, class_right);
      }
      if ((target_name == NULL || strlen(target_name) == 0 ||
           (hint.res_name && strcmp(hint.res_name, target_name) == 0)) &&
          (target_class == NULL || strlen(target_class) == 0 ||
           (hint.res_class && strcmp(hint.res_class, target_class) == 0))) {
        if (hint.res_name)
          XFree(hint.res_name);
        if (hint.res_class)
          XFree(hint.res_class);
        XFree(data);
        return windows[i];
      }
    }
    if (hint.res_name)
      XFree(hint.res_name);
    if (hint.res_class)
      XFree(hint.res_class);
  }
  XFree(data);
  return 0;
}

static Window get_active_window(Display *dpy, Window root) {
  Atom net_active_window = XInternAtom(dpy, NET_ACTIVE_WINDOW, True);
  if (net_active_window == None)
    return 0;

  Atom ret_type;
  int ret_format;
  unsigned long num_items, bytes_after;
  unsigned char *data = NULL;
  Status status = XGetWindowProperty(
      dpy, root, net_active_window, 0, (~0L), False, AnyPropertyType, &ret_type,
      &ret_format, &num_items, &bytes_after, &data);
  if (status != Success || ret_type == None || data == NULL) {
    if (data)
      XFree(data);
    return 0;
  }

  Window w = *(Window *)data;
  XFree(data);
  return w;
}

static void activate_window(Display *dpy, Window root, Window w) {
  Atom net_active_window = XInternAtom(dpy, NET_ACTIVE_WINDOW, False);
  if (net_active_window != None) {
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.xclient.type = ClientMessage;
    event.xclient.window = w;
    event.xclient.message_type = net_active_window;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2; /* 1 = application, 2 = pager */
    event.xclient.data.l[1] = CurrentTime;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;
    Status status = XSendEvent(dpy, root, False, mask, &event);
    if (status == Success)
      XFlush(dpy);
  }
}

static void store_previous_window(Display *dpy, Window root, Window w) {
  Atom window = XInternAtom(dpy, X11_ATOM_NAME, False);
  if (window != None) {
    unsigned long data[1] = {w};
    Status status = XChangeProperty(dpy, root, window, XA_WINDOW, 32,
                                    PropModeReplace, (unsigned char *)data, 1);
    if (status == Success)
      XFlush(dpy);
  }
}

static Window retrive_previous_window(Display *dpy, Window root) {
  Atom window = XInternAtom(dpy, X11_ATOM_NAME, False);
  if (window == None)
    return 0;

  Atom ret_type;
  int ret_format;
  unsigned long num_items, bytes_after;
  unsigned char *data = NULL;
  Status status =
      XGetWindowProperty(dpy, root, window, 0, 1, False, XA_WINDOW, &ret_type,
                         &ret_format, &num_items, &bytes_after, &data);
  if (status != Success || ret_type == None || data == NULL) {
    if (data)
      XFree(data);
    return 0;
  }

  Window win = *(Window *)data;
  XFree(data);
  return win;
}

static int print_usage(int rc, char *prog) {
  fprintf(stderr,
          "Usage: %s [-v|--verbose] [-h|--help] [-S|--no-store] [-w|--wait-ms] "
          "WM_NAME [WM_CLASS] [fallback_command...]\n",
          basename(prog));
  return rc;
}

int main(int argc, char **argv) {
  int opt;
  int opt_index = 0;
  static struct option long_opts[] = {
      {"help", no_argument, 0, 'h'},
      // {"class", required_argument, 0, 'c'},
      // {"name", required_argument, 0, 'n'},
      {"no-store", no_argument, 0, 'S'},
      {"verbose", no_argument, 0, 'v'},
      {"wait-ms", required_argument, 0, 'w'},
      {0, 0, 0, 0},
  };

  while ((opt = getopt_long(argc, argv, "hSvw:", long_opts, &opt_index)) !=
         -1) {
    switch (opt) {
    case 'v':
      options.verbose = 1;
      break;
    case 'S':
      options.store_previous = 0;
      break;
    case 'w':
      options.wait_ms = atoi(optarg);
      if (options.wait_ms < 0)
        die(202, "invalid value for --wait-ms: %s\n", optarg);
      break;
    case 'h':
      return print_usage(0, argv[0]);
    default:
      return print_usage(201, argv[0]);
    }
  }

  // fprintf(stderr, "argc=%d, optind=%d\n", argc, optind);
  if (argc == 1)
    return print_usage(0, argv[0]);

  const char *target_name = optind < argc ? argv[optind] : "";
  const char *target_class = optind + 1 < argc ? argv[optind + 1] : "";

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    const char *display_name = getenv("DISPLAY");
    die(203, "cannot open X display%s%s", display_name ? " " : "",
        display_name ? display_name : "");
  }

  Window root = DefaultRootWindow(dpy);
  Window win = find_window(dpy, root, target_name, target_class);
  if (win) {
    if (options.store_previous) {
      Window current = get_active_window(dpy, root);
      if (current == win) {
        win = retrive_previous_window(dpy, root);
        if (win) {
          warn("Switching back to 0x%0*lx", WINID_FMT_LEN, (unsigned long)win);
        } else
          win = current;
        current = 0;
      }
      store_previous_window(dpy, root, current);
    }
    activate_window(dpy, root, win);
    XCloseDisplay(dpy);
    return 0;
  }

  XCloseDisplay(dpy);

  if (argc == 2 && options.verbose)
    return 0;

  warn("%s.%s is not found", strlen(target_name) > 0 ? target_name : "\"\"",
       strlen(target_class) > 0 ? target_class : "\"\"");

  if (optind + 2 >= argc) {
    return 204;
  }

  if (options.verbose) {
    fprintf(stderr, "%s: executing:", PROG);
    for (int i = optind + 2; i < argc; i++) {
      fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr, "\n");
  }

  pid_t pid = fork();
  if (pid < 0)
    die(205, "fork() failed: %s", strerror(errno));

  if (!pid) { // child process
    execvp(argv[optind + 2], &argv[optind + 2]);
    die(206, "execvp() failed: %s", strerror(errno));
  }

  // parent process
  warn("Forked PID: %d", pid);
  if (options.wait_ms > 0 && (dpy = XOpenDisplay(NULL))) {
    warn("Waiting for %d ms", options.wait_ms);
    nsleep_ms(options.wait_ms);
    root = DefaultRootWindow(dpy);
    win = find_window(dpy, root, target_name, target_class);
    if (win) {
      warn("Activating 0x%0*lx", WINID_FMT_LEN, (unsigned long)win);
      activate_window(dpy, root, win);
    }
    XCloseDisplay(dpy);
  }

  return 0;
}
