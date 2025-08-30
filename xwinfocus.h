#include <stdio.h>

#include <X11/Xlib.h>

#ifndef PROG
#define PROG "xwinfocus"
#endif

#ifndef VERSION
#define VERSION "devel"
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "none"
#endif

#define DISPLAY_ENV_VAR "DISPLAY"

#define NET_ACTIVE_WINDOW "_NET_ACTIVE_WINDOW"
#define NET_CLIENT_LIST "_NET_CLIENT_LIST"

#ifndef X_ATOM_NAME
#define X_ATOM_NAME "_XWINFOCUS_PREVIOUS_WINDOW"
#endif

#define XA_WINDOW_FMT32 32

// Strictly speaking, WINID_FMT_LEN should be (int)(sizeof(unsigned long) * 2),
// but half the size is pretty much enough for output
#define WINID_FMT "0x%0*lx"
#define WINID_FMT_LEN (int)(sizeof(unsigned long))

#ifndef NULL_LABEL
#define NULL_LABEL "<null>"
#endif

#ifndef EMPTY_LABEL
#define EMPTY_LABEL "<empty>"
#endif

#ifndef LEFT_QUOTE
#define LEFT_QUOTE "\""
#endif

#ifndef RIGHT_QUOTE
#define RIGHT_QUOTE "\""
#endif

#ifndef HEADER_UNDERLINE
#define HEADER_UNDERLINE '-'
#endif

#ifndef MAX_BUFFER
#define MAX_BUFFER 256
#endif

#ifndef SPACE_LEN
#define SPACE_LEN 2
#endif

typedef struct {
  const char *class;   /* XClassHint.res_class */
  const char *name;    /* XClassHint.res_name */
  Bool store_previous; /* store current active window id into a property */
  Bool list;
  Bool verbose;
  int wait_ms;
} options_t;

__attribute__((noreturn, format(printf, 2, 3))) static void
die(int, const char *, ...);

__attribute__((format(printf, 1, 2))) static void warn(const char *, ...);

static void nsleep_ms(int);

static unsigned long window_property(Display *, Window, const char *,
                                     unsigned char **);
static Window find_window(Display *, Window, const char *, const char *);
static Window active_window(Display *, Window);
static Window retrieve_previous_window(Display *, Window);
static void store_previous_window(Display *, Window, Window);
static void activate_window(Display *, Window, Window);
static int print_version(int);
static int print_usage(int);
static size_t option_string(const struct option *, char *, size_t);

#ifndef NO_LIST_WINDOWS
static inline size_t min(size_t, size_t);
static inline size_t max(size_t, size_t);

static size_t fringe(const char *, char *, size_t);

static void underline(FILE *, char *, size_t, char *, size_t, char *, size_t,
                      int);

static void list_windows(FILE *, Display *, Window);
#endif
