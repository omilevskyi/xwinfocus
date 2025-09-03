#ifndef NO_LIST_WINDOWS

#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>

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

size_t fringe(const char *, char *, size_t);
#endif
