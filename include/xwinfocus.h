#if defined(NO_LIST_WINDOWS) ||                                                \
    (!defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__))
#define _POSIX_C_SOURCE 200809L
#endif

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

#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR '-'
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
static Window window_property_value(Display *, Window, const char *);
static Window active_window(Display *, Window);
static Window retrieve_previous_window(Display *, Window);
static void store_previous_window(Display *, Window, Window);
static Window find_window(Display *, Window, const char *, const char *);
static void activate_window(Display *, Window, Window);
static int print_version(int);
static int print_usage(int);

#ifndef NO_LIST_WINDOWS
static inline size_t min(size_t, size_t);
static inline size_t max(size_t, size_t);
static void fprint_delimiter(FILE *, char *, size_t, char *, size_t, char *,
                             size_t, int);
static void list_windows(FILE *, Display *, Window);
#endif
