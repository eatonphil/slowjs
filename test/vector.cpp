#include <stdint.h>

#include "gtest/gtest.h"

extern "C" {
#include "slowjs/vector.h"
}

DECLARE_VECTOR(uint64_t);

TEST(vector, copy_from_array) {
  const char test[] = "foo";
  vector_char s = {0};
  vector_error err = E_VECTOR_OK;
  uint64_t i = 0;

  ASSERT_EQ(sizeof(test), 4);
  err = vector_char_copy(&s, (char *)test, sizeof(test));
  ASSERT_EQ(E_VECTOR_OK, err);

  ASSERT_EQ(s.index, sizeof(test));

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(s.elements[i], test[i]);
  }

  vector_char_free(&s);
}

TEST(vector, copy_from_vector) {
  const uint64_t test[] = {2, 3, 4};
  vector_uint64_t s = {0}, copy = {0};
  vector_error err = E_VECTOR_OK;
  uint64_t i = 0;

  err = vector_uint64_t_copy(&s, (uint64_t *)test,
                             sizeof(test) / sizeof(test[0]));
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(3, s.index);

  err = vector_uint64_t_copy(&copy, s.elements, s.index);
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(3, s.index);

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(copy.elements[i], test[i]);
  }

  vector_uint64_t_free(&s);
  vector_uint64_t_free(&copy);
}

TEST(vector, copy_resize_when_neccessary) {
  const uint64_t test[] = {2, 3, 4};
  vector_uint64_t s = {0};
  vector_error err = E_VECTOR_OK;
  uint64_t i = 0;

  ASSERT_EQ(0, s.size);

  // Copy from array larger than size
  err = vector_uint64_t_copy(&s, (uint64_t *)test,
                             sizeof(test) / sizeof(test[0]));
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(3, s.index);
  ASSERT_LE(3, s.size);

  vector_uint64_t_free(&s);
}

TEST(vector, push) {
  const uint64_t test[] = {1, 8, 20};
  uint64_t i = 0;
  vector_uint64_t s = {0};

  for (i = 0; i < (sizeof(test) / sizeof(test[0])); i++) {
    vector_uint64_t_push(&s, test[i]);
    ASSERT_EQ(i + 1, s.index);
  }

  ASSERT_EQ(3, s.index);
  ASSERT_LE(3, s.size);

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(test[i], s.elements[i]);
  }

  vector_uint64_t_free(&s);
}

TEST(vector, push_resize_when_neccessary) {
  uint64_t i = 0;
  vector_uint64_t s = {0};
  uint64_t bigger_size = (s.size + 3) * 2; // Add 3 in case size is 0
  ASSERT_GT(bigger_size, 0);

  for (i = 0; i < bigger_size; i++) {
    vector_uint64_t_push(&s, i);
  }

  ASSERT_EQ(bigger_size, s.index);
  ASSERT_LE(bigger_size, s.size);

  for (i = 0; i < s.index; i++) {
    ASSERT_EQ(i, s.elements[i]);
  }

  vector_uint64_t_free(&s);
}

TEST(vector, pop) {
  const uint64_t test[] = {99, 54, 12};
  vector_uint64_t s = {0};
  vector_error err = E_VECTOR_OK;
  uint64_t test_element = 0;

  err = vector_uint64_t_copy(&s, (uint64_t *)test,
                             sizeof(test) / sizeof(test[0]));
  ASSERT_EQ(E_VECTOR_OK, err);

  err = vector_uint64_t_pop(&s, &test_element);
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(12, test_element);

  err = vector_uint64_t_pop(&s, &test_element);
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(54, test_element);

  err = vector_uint64_t_pop(&s, &test_element);
  ASSERT_EQ(E_VECTOR_OK, err);
  ASSERT_EQ(99, test_element);

  err = vector_uint64_t_pop(&s, &test_element);
  ASSERT_EQ(E_VECTOR_POP, err);

  vector_uint64_t_free(&s);
}
