#include <X11/Xlib.h>
#include <stdio.h>

__attribute__((noreturn, format(printf, 2, 3))) static void
die(int, const char *, ...);

__attribute__((format(printf, 1, 2))) static void warn(const char *, ...);

static void nsleep_ms(int);

static inline size_t min(size_t, size_t);
static inline size_t max(size_t, size_t);

static size_t fringe_len(const char *);
static const char *fringe(char *, size_t, const char *);

static void underline(FILE *, char *, size_t, char *, size_t, char *, size_t,
                      int);

static unsigned long get_window_property(Display *, Window, const char *,
                                         unsigned char **);

static Window find_window(Display *, Window, const char *, const char *);
static void list_windows(FILE *, Display *, Window);
static Window get_active_window(Display *, Window);
static Window retrieve_previous_window(Display *, Window);
static void store_previous_window(Display *, Window, Window);
static void activate_window(Display *, Window, Window);
static int print_version(int);
static int print_usage(int);
