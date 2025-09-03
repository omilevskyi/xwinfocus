#if defined(NO_LIST_WINDOWS) ||                                                \
    (!defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__))
#define _POSIX_C_SOURCE 200809L
#endif

#include <getopt.h>
#include <stdio.h>

size_t option_string(const struct option *, char *, size_t);
