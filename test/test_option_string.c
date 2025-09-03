#include "include/option_string.h"

#include <getopt.h>

#include <criterion/criterion.h>

Test(option_string_tests, null_dst_test) {
  struct option opts[] = {
      {"help", required_argument, NULL, 'h'},
      {"output", optional_argument, NULL, 'o'},
      {NULL, 0, NULL, 0},
  };

  struct {
    size_t length;
  } expected = {5};

  size_t len = option_string(opts, NULL, 0);
  cr_assert_eq(len, expected.length,
               "Expected length %zu when dst is NULL, got %zu", expected.length,
               len);
}

Test(option_string_tests, basic_options_test) {
  struct option opts[] = {
      {"help", required_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'v'},
      {"output", optional_argument, NULL, 'o'},
      {NULL, 0, NULL, 0},
  };

  struct {
    size_t length;
    const char *string;
    char buffer[10];
  } expected = {6, "h:vo::", ""};

  size_t len = option_string(opts, expected.buffer, sizeof(expected.buffer));

  cr_assert_eq(len, expected.length, "Expected length %zu, got %zu",
               expected.length, len);

  cr_assert_str_eq(expected.buffer, expected.string,
                   "Expected string '%s', got '%s'", expected.string,
                   expected.buffer);
}

Test(option_string_tests, working_options_test) {
  struct option opts[] = {
      {"help", no_argument, 0, 'h'},
      {"class", required_argument, 0, 'c'},
      {"list", no_argument, 0, 'l'},
      {"name", required_argument, 0, 'n'},
      {"no-store", no_argument, 0, 'S'},
      {"verbose", no_argument, 0, 'v'},
      {"version", no_argument, 0, 1},
      {"wait-ms", required_argument, 0, 'w'},
      {NULL, 0, NULL, 0},
  };

  struct {
    size_t length;
    const char *string;
    char buffer[20];
  } expected = {10, "hc:ln:Svw:", ""};

  size_t len = option_string(opts, expected.buffer, sizeof(expected.buffer));

  cr_assert_eq(len, expected.length, "Expected length %zu, got %zu",
               expected.length, len);

  cr_assert_str_eq(expected.buffer, expected.string,
                   "Expected string '%s', got '%s'", expected.string,
                   expected.buffer);
}
