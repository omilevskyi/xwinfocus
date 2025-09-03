#ifndef NO_LIST_WINDOWS
#include "include/fringe.h"

#include <ctype.h>
#include <string.h>

size_t fringe(const char *str, char *buf, size_t buf_size) {
  if (!str) {
    if (buf)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
      strlcpy(buf, NULL_LABEL, buf_size);
#else
      snprintf(buf, buf_size, "%s", NULL_LABEL);
#endif
    return 0;
  }

  if (!*str) {
    if (buf)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
      strlcpy(buf, EMPTY_LABEL, buf_size);
#else
      snprintf(buf, buf_size, "%s", EMPTY_LABEL);
#endif
    return 0;
  }

  size_t result = strlen(str);
  for (size_t i = 0; i < result; ++i)
    if (isblank(str[i])) {
      if (buf) {
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
        strlcpy(buf, LEFT_QUOTE, buf_size);
        strlcat(buf, str, buf_size);
        strlcat(buf, RIGHT_QUOTE, buf_size);
#else
        snprintf(buf, buf_size, LEFT_QUOTE "%s" RIGHT_QUOTE, str);
#endif
      }
      return sizeof LEFT_QUOTE - 1 + result + sizeof RIGHT_QUOTE - 1;
    }

  if (buf)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    strlcpy(buf, str, buf_size);
#else
    snprintf(buf, buf_size, "%s", str);
#endif

  return result;
}
#endif
