#include "include/option_string.h"

size_t option_string(const struct option *opts, char *dst, size_t dst_size) {
  size_t pos = 0;
  dst_size = dst ? dst_size - 1 : (size_t)(-1L);

  for (int i = 0; opts[i].name != NULL && pos < dst_size; ++i)
    if (opts[i].val > 1) { // only short forms are processed
      if (dst)
        dst[pos] = (char)opts[i].val;
      pos++;
      if (pos < dst_size)
        switch (opts[i].has_arg) {
        case required_argument:
          if (dst)
            dst[pos] = ':';
          pos++;
          break;
        case optional_argument:
          if (dst)
            dst[pos] = ':';
          pos++;
          if (pos < dst_size) {
            if (dst)
              dst[pos] = ':';
            pos++;
          }
        }
    }

  if (dst)
    dst[pos] = '\0';

  return pos;
}
