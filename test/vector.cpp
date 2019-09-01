#include "gtest/gtest.h"

extern "C" {
#include "slowjs/vector.h"
}

TEST(vector, copy_from_array) {
  const char test[] = "foo";
  vector_char s = {0};
  vector_error err = E_VECTOR_OK;
  int i = 0;

  ASSERT_EQ(sizeof(test), 4);
  err = vector_char_copy(&s, (char *)test, sizeof(test));
  ASSERT_EQ(E_VECTOR_OK, err);

  ASSERT_EQ(s.index, sizeof(test));

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(s.elements[i], test[i]);
  }
}

TEST(vector, copy_from_vector) {
  const char test[] = "foo";
  vector_char s = {0}, copy = {0};
  vector_error err = E_VECTOR_OK;
  int i = 0;

  err = vector_char_copy(&s, (char *)test, sizeof(test));
  ASSERT_EQ(E_VECTOR_OK, err);

  err = vector_char_copy(&copy, s.elements, s.index);
  ASSERT_EQ(E_VECTOR_OK, err);

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(copy.elements[i], test[i]);
  }
}
