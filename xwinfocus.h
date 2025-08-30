#include <stdio.h>

#include <X11/Xlib.h>

#define PROG "xwinfocus"

#ifndef VERSION
#define VERSION "devel"
#endif

#ifndef COMMIT_HASH
#define COMMIT_HASH "none"
#endif

#define NET_ACTIVE_WINDOW "_NET_ACTIVE_WINDOW"
#define NET_CLIENT_LIST "_NET_CLIENT_LIST"
#define X_ATOM_NAME "_XWINFOCUS_PREVIOUS_WINDOW"

#define XA_WINDOW_FMT32 32

// Strictly speaking, WINID_FMT_LEN should be (int)(sizeof(unsigned long) * 2),
// but half the size is pretty much enough for output
#define WINID_FMT "0x%0*lx"
#define WINID_FMT_LEN (int)(sizeof(unsigned long))
#define NULL_LABEL "<null>"
#define EMPTY_LABEL "<empty>"
#define LEFT_QUOTE "\""
#define RIGHT_QUOTE "\""
#define HEADER_UNDERLINE '-'

#define MAX_BUFFER 512
#define SPACE_LEN 2

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
