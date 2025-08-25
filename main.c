#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define NET_ACTIVE_WINDOW "_NET_ACTIVE_WINDOW"
#define NET_CLIENT_LIST "_NET_CLIENT_LIST"

#define X11_ATOM_NAME "_XWINFOCUS_PREVIOUS_WINDOW"

int static verbose = 0;
int static store_window_id = 1;
int static wait_ms = 0;

static const char *fringe(const char *, char *, char *);
static const char *fringe(const char *str, char *left, char *right) {
  if (str) {
    *left = *right = '"';
    return str;
  }
  *left = '<';
  *right = '>';
  return "null";
}

Window static find_window(Display *, Window, const char *, const char *);
Window static find_window(Display *dpy, Window root, const char *target_name,
                          const char *target_class) {
  Atom netClientList = XInternAtom(dpy, NET_CLIENT_LIST, True);
  Atom actualType;
  int actualFormat;
  unsigned long nItems, bytesAfter;
  unsigned char *data = NULL;

  if (XGetWindowProperty(dpy, root, netClientList, 0, (~0L), False, XA_WINDOW,
                         &actualType, &actualFormat, &nItems, &bytesAfter,
                         &data) != Success ||
      !data)
    return 0;

  Window *windows = (Window *)data;
  for (unsigned long i = 0; i < nItems; i++) {
    XClassHint hint;
    if (XGetClassHint(dpy, windows[i], &hint)) {
      if (verbose) {
        char name_left, name_right, class_left, class_right;
        const char *name_body = fringe(hint.res_name, &name_left, &name_right);
        const char *class_body =
            fringe(hint.res_class, &class_left, &class_right);
        fprintf(stderr,
                "Window: 0x%08lx\tres_name: %c%s%c\tres_class: %c%s%c\n",
                windows[i], name_left, name_body, name_right, class_left,
                class_body, class_right);
      }
      if (hint.res_name && hint.res_class &&
          strcmp(hint.res_name, target_name) == 0 &&
          (target_class == NULL || strlen(target_class) == 0 ||
           strcmp(hint.res_class, target_class) == 0)) {
        if (hint.res_name)
          XFree(hint.res_name);
        if (hint.res_class)
          XFree(hint.res_class);
        XFree(data);
        return windows[i];
      }
      if (hint.res_name)
        XFree(hint.res_name);
      if (hint.res_class)
        XFree(hint.res_class);
    }
  }
  XFree(data);
  return 0;
}

Window static get_active_window(Display *, Window);
Window static get_active_window(Display *dpy, Window root) {
  Atom activeAtom = XInternAtom(dpy, NET_ACTIVE_WINDOW, True);
  Atom actualType;
  int actualFormat;
  unsigned long nItems, bytesAfter;
  unsigned char *data = NULL;

  if (XGetWindowProperty(dpy, root, activeAtom, 0, (~0L), False,
                         AnyPropertyType, &actualType, &actualFormat, &nItems,
                         &bytesAfter, &data) != Success ||
      !data)
    return 0;

  Window active = *(Window *)data;
  XFree(data);
  return active;
}

void static activate_window(Display *, Window, Window);
void static activate_window(Display *dpy, Window root, Window win) {
  XEvent event = {0};
  event.xclient.type = ClientMessage;
  event.xclient.window = win;
  event.xclient.message_type = XInternAtom(dpy, NET_ACTIVE_WINDOW, False);
  event.xclient.format = 32;
  event.xclient.data.l[0] = 1;           // source indication (1 = application)
  event.xclient.data.l[1] = CurrentTime; // timestamp
  // event.xclient.data.l[2] = event.xclient.data.l[3] = event.xclient.data.l[4]
  // = 0; // unused

  XSendEvent(dpy, root, False,
             SubstructureRedirectMask | SubstructureNotifyMask, &event);
  XFlush(dpy);
}

void static store_previous_window(Display *, Window, Window);
void static store_previous_window(Display *dpy, Window root, Window win) {
  Atom atom = XInternAtom(dpy, X11_ATOM_NAME, False);
  unsigned long data[1] = {win};
  XChangeProperty(dpy, root, atom, XA_WINDOW, 32, PropModeReplace,
                  (unsigned char *)data, 1);
  XFlush(dpy);
}

Window static retrive_previous_window(Display *, Window);
Window static retrive_previous_window(Display *dpy, Window root) {
  Atom atom = XInternAtom(dpy, X11_ATOM_NAME, False);
  Atom actualType;
  int actualFormat;
  unsigned long nItems, bytesAfter;
  unsigned char *data = NULL;

  if (XGetWindowProperty(dpy, root, atom, 0, 1, False, XA_WINDOW, &actualType,
                         &actualFormat, &nItems, &bytesAfter,
                         &data) != Success ||
      !data)
    return 0;

  Window win = *(Window *)data;
  XFree(data);
  return win;
}

int static print_usage(int rc, char *prog) {
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
      {"no-store", no_argument, 0, 'S'},
      {"verbose", no_argument, 0, 'v'},
      {"wait-ms", required_argument, 0, 'w'},
      {0, 0, 0, 0},
  };

  while ((opt = getopt_long(argc, argv, "hSvw", long_opts, &opt_index)) != -1) {
    switch (opt) {
    case 'v':
      verbose = 1;
      break;
    case 'S':
      store_window_id = 0;
      break;
    case 'w':
      wait_ms = atoi(optarg);
      if (wait_ms < 0) {
        fprintf(stderr, "Invalid value for --wait-ms: %s\n", optarg);
        return 202;
      }
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
    fprintf(stderr, "Cannot open display\n");
    return 203;
  }

  Window root = DefaultRootWindow(dpy);
  Window win = find_window(dpy, root, target_name, target_class);
  if (win) {
    if (store_window_id) {
      Window current = get_active_window(dpy, root);
      if (current == win) {
        win = retrive_previous_window(dpy, root);
        if (win) {
          if (verbose)
            fprintf(stderr, "Switching back to 0x%08lx.\n", win);
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

  if (argc == 2 && verbose)
    return 0;

  if (verbose)
    fprintf(stderr, "%s.%s is not found.\n",
            strlen(target_name) > 0 ? target_name : "\"\"",
            strlen(target_class) > 0 ? target_class : "\"\"");

  if (optind + 2 >= argc) {
    return 204;
  }

  if (verbose) {
    fprintf(stderr, "Executing:");
    for (int i = optind + 2; i < argc; i++) {
      fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr, "\n");
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return 205;
  }

  if (!pid) { // child process
    execvp(argv[optind + 2], &argv[optind + 2]);
    perror("execvp failed");
    return 206;
  }

  // parent process
  if (verbose)
    fprintf(stderr, "Forked PID: %d\n", pid);

  if (wait_ms > 0 && (dpy = XOpenDisplay(NULL))) {
    if (verbose)
      fprintf(stderr, "Waiting for %d ms.\n", wait_ms);
    root = DefaultRootWindow(dpy);
    usleep(wait_ms * 1000); // convert ms to microseconds
    win = find_window(dpy, root, target_name, target_class);
    if (win) {
      if (verbose)
        fprintf(stderr, "Activating 0x%08lx\n", win);
      activate_window(dpy, root, win);
    }
    XCloseDisplay(dpy);
  }

  return 0;
}
