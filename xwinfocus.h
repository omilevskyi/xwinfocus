#include <X11/Xlib.h>

static void die(int, const char *, ...)
    __attribute__((noreturn, format(printf, 2, 3)));

static void warn(const char *, ...);
static void nsleep_ms(int);

static const char *fringe(const char *, char *, char *);
static Window find_window(Display *, Window, const char *, const char *);
static Window get_active_window(Display *, Window);
static void activate_window(Display *, Window, Window);
static void store_previous_window(Display *, Window, Window);
static Window retrive_previous_window(Display *, Window);
static int print_usage(int, char *);
