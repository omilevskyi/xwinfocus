#include "include/xwinfocus.h"

#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "include/fringe.h"
#include "include/option_string.h"

static options_t options = {"", "", True, False, False, 0};

static void die(int rc, const char *fmt, ...) {
  if (fmt) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "%s: ", PROG);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
  }
  exit(rc);
}

static void warn(const char *fmt, ...) {
  if (options.verbose) {
    fprintf(stderr, "%s: ", PROG);
    if (fmt) {
      va_list ap;
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
    } else
      fprintf(stderr, NULL_LABEL);
    fprintf(stderr, "\n");
  }
}

static void nsleep_ms(int ms) {
  if (ms > 0) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
      /* retry */
    }
  }
}

#ifndef NO_LIST_WINDOWS
static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }
static inline size_t max(size_t a, size_t b) { return a > b ? a : b; }

static void fprint_delimiter(FILE *f, char *whdr, size_t whdr_size, char *name,
                             size_t name_size, char *class, size_t class_size,
                             int nspace) {
  whdr[whdr_size - 1] = name[name_size - 1] = class[class_size - 1] = '\0';
  fprintf(f, "%s%*s%s%*s%s\n",
          (char *)memset(whdr, DELIMITER_CHAR, whdr_size - 1), nspace, "",
          (char *)memset(name, DELIMITER_CHAR, name_size - 1), nspace, "",
          (char *)memset(class, DELIMITER_CHAR, class_size - 1));
}
#endif

/* fetch property on a window; caller must XFree(*out) */
static unsigned long window_property(Display *dpy, Window w,
                                     const char *atom_name,
                                     unsigned char **out) {
  unsigned long nitems = 0;
  unsigned char *data = NULL;
  Atom prop = XInternAtom(dpy, atom_name, True);
  if (prop != None) {
    Atom actual_type;
    int actual_format;
    unsigned long bytes_after;
    Status status = XGetWindowProperty(dpy, w, prop, 0L, (~0L), False,
                                       XA_WINDOW, &actual_type, &actual_format,
                                       &nitems, &bytes_after, &data);
    if (status != Success || actual_type != XA_WINDOW ||
        actual_format != XA_WINDOW_FMT32 || data == NULL) {
      nitems = 0;
      if (data)
        XFree(data);
      data = NULL;
    }
  }
  if (out)
    *out = data;
  return nitems;
}

static Window find_window(Display *dpy, Window root, const char *target_name,
                          const char *target_class) {
  Window w = 0;
  unsigned char *data = NULL;
  unsigned long n = window_property(dpy, root, NET_CLIENT_LIST, &data);
  if (data) {
    Window *windows = (Window *)data;
    XClassHint hint;
    for (unsigned long i = 0; i < n && !w; i++)
      if (XGetClassHint(dpy, windows[i], memset(&hint, 0, sizeof hint))) {
        if ((target_name == NULL || *target_name == '\0' ||
             (hint.res_name && strcmp(hint.res_name, target_name) == 0)) &&
            (target_class == NULL || *target_class == '\0' ||
             (hint.res_class && strcmp(hint.res_class, target_class) == 0)))
          w = windows[i];
        if (hint.res_name)
          XFree(hint.res_name);
        if (hint.res_class)
          XFree(hint.res_class);
      }
    XFree(data);
  }
  return w;
}

#ifndef NO_LIST_WINDOWS
static void list_windows(FILE *f, Display *dpy, Window root) {
  unsigned char *data = NULL;
  unsigned long n = window_property(dpy, root, NET_CLIENT_LIST, &data);
  if (!n) {
    if (data)
      XFree(data);

    warn("fallback to using XQueryTree()");
    Window parent;
    data = NULL;
    unsigned int uintn = 0;
    if (XQueryTree(dpy, root, &root, &parent, (Window **)&data, &uintn) == 0) {
      warn("XQueryTree() failed");
      return;
    }
    n = (unsigned long)uintn;
  }

  if (data) {
    if (n) {
      Window *windows = (Window *)data;
      XClassHint hint;
      size_t name_len, class_len;
      name_len = class_len =
          max(fringe(NULL_LABEL, NULL, 0), fringe(EMPTY_LABEL, NULL, 0));

      for (unsigned long i = 0; i < n; i++)
        if (XGetClassHint(dpy, windows[i], memset(&hint, 0, sizeof hint))) {
          if (hint.res_name) {
            name_len = max(name_len, fringe(hint.res_name, NULL, 0));
            XFree(hint.res_name);
          }
          if (hint.res_class) {
            class_len = max(class_len, fringe(hint.res_class, NULL, 0));
            XFree(hint.res_class);
          }
        }

      name_len = min(name_len, MAX_BUFFER - 1);
      class_len = min(class_len, MAX_BUFFER - 1);

      char whdr[WINID_FMT_LEN + 2 + 1], name[name_len + 1],
          class[class_len + 1];

      fprint_delimiter(f, whdr, sizeof whdr, name, sizeof name, class,
                       sizeof class, SPACE_LEN);

      fprintf(f, "%-*.*s%*s%-*.*s%*s%-*.*s\n", WINID_FMT_LEN + 2,
              WINID_FMT_LEN + 2, "Window ID", SPACE_LEN, "", (int)name_len,
              (int)name_len, "Name", SPACE_LEN, "", (int)class_len,
              (int)class_len, "Class");

      fprint_delimiter(f, whdr, sizeof whdr, name, sizeof name, class,
                       sizeof class, SPACE_LEN);

      for (unsigned long i = 0; i < n; i++)
        if (XGetClassHint(dpy, windows[i], memset(&hint, 0, sizeof hint))) {
          fringe(hint.res_name, name, sizeof name);
          fringe(hint.res_class, class, sizeof class);
          fprintf(f, WINID_FMT "%*s%-*.*s%*s%-*.*s\n", WINID_FMT_LEN,
                  (unsigned long)windows[i], SPACE_LEN, "", (int)name_len,
                  (int)name_len, name, SPACE_LEN, "", (int)class_len,
                  (int)class_len, class);
          if (hint.res_name)
            XFree(hint.res_name);
          if (hint.res_class)
            XFree(hint.res_class);
        }

      fprint_delimiter(f, whdr, sizeof whdr, name, sizeof name, class,
                       sizeof class, SPACE_LEN);
    }
    XFree(data);
  }
}
#endif

static Window window_property_value(Display *dpy, Window root,
                                    const char *atom_name) {
  Window w = 0;
  unsigned char *data = NULL;
  unsigned long n = window_property(dpy, root, atom_name, &data);
  if (data) {
    if (n)
      memcpy(&w, data, sizeof w);
    XFree(data);
  }
  return w;
}
static Window active_window(Display *dpy, Window root) {
  return window_property_value(dpy, root, NET_ACTIVE_WINDOW);
}

static Window retrieve_previous_window(Display *dpy, Window root) {
  return window_property_value(dpy, root, X_ATOM_NAME);
}

static void store_previous_window(Display *dpy, Window root, Window w) {
  Atom window = XInternAtom(dpy, X_ATOM_NAME, False);
  if (window != None) {
    unsigned long data[1] = {w};
    XChangeProperty(dpy, root, window, XA_WINDOW, XA_WINDOW_FMT32,
                    PropModeReplace, (unsigned char *)data,
                    sizeof data / sizeof *data); /* / sizeof(unsigned long) */
    XFlush(dpy);
  }
}

static void activate_window(Display *dpy, Window root, Window w) {
  Atom window = XInternAtom(dpy, NET_ACTIVE_WINDOW, False);
  if (window != None) {
    XEvent event;
    memset(&event, 0, sizeof event);
    event.xclient.type = ClientMessage;
    event.xclient.window = w;
    event.xclient.message_type = window;
    event.xclient.format = XA_WINDOW_FMT32;
    event.xclient.data.l[0] = 2; /* 1 = application, 2 = pager */
    event.xclient.data.l[1] = CurrentTime;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;
    if (XSendEvent(dpy, root, False, mask, &event) == Success)
      XFlush(dpy);
  }
}

static int print_version(int rc) {
  printf("%s %s (%s)\n", PROG, VERSION, COMMIT_HASH);
  return rc;
}

static int print_usage(int rc) {
  printf("Usage: %s [options] [fallback command with parameters]\n"
         "\n"
         "Activate X11 window or run command if window is not found.\n"
         "\n"
         "Options:\n"
         "  -c, --class          Match window by XClassHint.res_class\n"
         "  -n, --name           Match window by XClassHint.res_name\n"
#ifndef NO_LIST_WINDOWS
         "  -l, --list           List all windows\n"
#endif
         "  -S, --no-store       Do not store current active window id "
         "in " X_ATOM_NAME " window property\n"
         "  -v, --verbose        Verbose text output\n"
         "  -w, --wait-ms <ms>   Wait this many milliseconds after fallback "
         "command is run and then activate window\n"
         "      --version        Show version and exit\n"
         "  -h, --help           Show this help and exit\n"
         "\n"
         "Examples:\n"
#ifndef NO_LIST_WINDOWS
         "  %s --list\t# show all windows found\n"
#endif
         "  %s -n chromium-browser -- chrome --no-proxy-server google.com\n",
#ifndef NO_LIST_WINDOWS
         PROG,
#endif
         PROG, PROG);
  return rc;
}

int main(int argc, char **argv) {
  static struct option long_opts[] = {
      {"help", no_argument, NULL, 'h'},
      {"class", required_argument, NULL, 'c'},
#ifndef NO_LIST_WINDOWS
      {"list", no_argument, NULL, 'l'},
#endif
      {"name", required_argument, NULL, 'n'},
      {"no-store", no_argument, NULL, 'S'},
      {"verbose", no_argument, NULL, 'v'},
      {"version", no_argument, NULL, 1},
      {"wait-ms", required_argument, NULL, 'w'},
      {NULL, 0, NULL, 0},
  };

  if (argc < 2)
    return print_usage(EXIT_SUCCESS);

  int opt, opt_idx = 0;
  char opt_string[option_string(long_opts, NULL, 0) + 1];
  option_string(long_opts, opt_string, sizeof opt_string); /* "hc:ln:Svw:" */

  while ((opt = getopt_long(argc, argv, opt_string, long_opts, &opt_idx)) !=
         -1) {
    switch (opt) {
    case 'h':
      return print_usage(EXIT_SUCCESS);
    case 'c':
      if (optarg)
        options.class = optarg;
      break;
#ifndef NO_LIST_WINDOWS
    case 'l':
      options.list = True;
      break;
#endif
    case 'n':
      if (optarg)
        options.name = optarg;
      break;
    case 'S':
      options.store_previous = False;
      break;
    case 'v':
      options.verbose = True;
      break;
    case 'w':
      options.wait_ms = atoi(optarg);
      if (options.wait_ms < 0)
        die(202, "invalid value for --wait-ms: %s\n", optarg);
      break;
    case 1: /* --version */
      return print_version(EXIT_SUCCESS);
    default:
      warn("invalid option");
      return print_usage(201);
    }
  }

  const char *display = getenv(DISPLAY_ENV_VAR);
  Display *dpy = XOpenDisplay(display);
  if (!dpy)
    die(203, "cannot open X display%s%s", display ? " " : "",
        display ? display : "");

  Window root = XDefaultRootWindow(dpy);

#ifndef NO_LIST_WINDOWS
  if (options.list) {
    list_windows(stdout, dpy, root);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
  }
#endif

  Window win = find_window(dpy, root, options.name, options.class);
  if (win) {
    if (options.store_previous) {
      Window current = active_window(dpy, root);
      if (current == win) {
        win = retrieve_previous_window(dpy, root);
        if (win)
          warn("Switching back to " WINID_FMT, WINID_FMT_LEN,
               (unsigned long)win);
        else
          win = current;
        current = 0;
      }
      store_previous_window(dpy, root, current);
    }
    activate_window(dpy, root, win);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
  }

  XCloseDisplay(dpy);

  warn("%s.%s is not found", *options.name ? options.name : "\"\"",
       *options.class ? options.class : "\"\"");

  if (optind >= argc) {
    warn("fallback command is not found");
    die(204, NULL);
  }

  if (options.verbose) {
    fprintf(stderr, "%s: executing:", PROG);
    for (int i = optind; i < argc; i++)
      fprintf(stderr, " %s", argv[i]);
    fprintf(stderr, "\n");
  }

  pid_t pid = fork();
  if (pid < 0)
    die(205, "fork() failed: %s", strerror(errno));

  if (!pid) { /* child process */
    execvp(argv[optind], &argv[optind]);
    die(206, "execvp() failed: %s", strerror(errno));
  }

  /* parent process */
  warn("Forked PID: %d", pid);

  /* open fresh Display after child starts */
  if (options.wait_ms > 0 && (dpy = XOpenDisplay(display))) {
    warn("Waiting for %d ms", options.wait_ms);
    nsleep_ms(options.wait_ms);
    root = XDefaultRootWindow(dpy);
    win = find_window(dpy, root, options.name, options.class);
    if (win) {
      warn("Activating " WINID_FMT, WINID_FMT_LEN, (unsigned long)win);
      activate_window(dpy, root, win);
    } else
      warn("%s.%s is not found", *options.name ? options.name : "\"\"",
           *options.class ? options.class : "\"\"");
    XCloseDisplay(dpy);
  }

  return EXIT_SUCCESS;
}
