#include "include/fringe.h"

#include <string.h>

#include <criterion/criterion.h>

Test(fringe_tests, null_input) {
  char buf[128] = {0};
  size_t len = fringe(NULL, buf, sizeof(buf));

  cr_assert_eq(len, 0, "Expected length 0 for NULL input");
  cr_assert_str_eq(buf, NULL_LABEL, "Expected buffer to contain " NULL_LABEL);
}

Test(fringe_tests, empty_string_input) {
  char buf[128] = {0};
  size_t len = fringe("", buf, sizeof(buf));

  cr_assert_eq(len, 0, "Expected length 0 for empty string");
  cr_assert_str_eq(buf, EMPTY_LABEL, "Expected buffer to contain " EMPTY_LABEL);
}

Test(fringe_tests, string_with_spaces) {
  const char *input = "hello world";
  char buf[128] = {0};
  size_t len = fringe(input, buf, sizeof(buf));

  char expected[128] = LEFT_QUOTE "hello world" RIGHT_QUOTE;
  size_t expected_len = strlen(expected);

  cr_assert_eq(len, expected_len, "Expected quoted length");
  cr_assert_str_eq(buf, expected, "Expected quoted string in buffer");
}

Test(fringe_tests, string_without_spaces) {
  const char *input = "helloworld";
  char buf[128] = {0};
  size_t len = fringe(input, buf, sizeof(buf));

  cr_assert_eq(len, strlen(input), "Expected length equal to input length");
  cr_assert_str_eq(buf, input, "Expected buffer to contain input string");
}

Test(fringe_tests, buffer_null) {
  const char *input = "hello world";
  size_t expected_len =
      sizeof LEFT_QUOTE - 1 + strlen(input) + sizeof RIGHT_QUOTE - 1;

  size_t len = fringe(input, NULL, 0);

  cr_assert_eq(len, expected_len,
               "Expected correct length even with NULL buffer");
}

Test(fringe_tests, buffer_too_small) {
  const char *input = "hello world";
  char buf[4] = {0}; // intentionally small

  fringe(input, buf, sizeof(buf));

  // Just check that buffer is null-terminated
  cr_assert(buf[sizeof(buf) - 1] == '\0', "Buffer should be null-terminated");
}
