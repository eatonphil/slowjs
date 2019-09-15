#include "gtest/gtest.h"

extern "C" {
#include "slowjs/lex.h"
}

TEST(lex, addition) {
  vector_token tokens = {0};
  vector_char source = {0};
  const char raw_source[] = "a + 1";
  const char *expected[] = {
      "1",
      "+",
      "a",
  };
  int i = 0;
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_EQ(sizeof expected / sizeof expected[0], tokens.index);

  for (i = 0; i < tokens.index; i++) {
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}

TEST(lex, ret) {
  vector_token tokens = {0};
  vector_char source = {0};
  const char raw_source[] = "return 1;";
  const char *expected[] = {
      ";",
      "1",
      "return",
  };
  int i = 0;
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_EQ(3, tokens.index);

  for (i = 0; i < tokens.index; i++) {
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}

TEST(lex, function) {
  const char raw_source[] = "function main() { return a+1; }";
  const char *expected[] = {
      "}", ";", "1", "+", "a", "return", "{", ")", "(", "main", "function",
  };
  vector_token tokens = {0};
  vector_char source = {0};
  int i = 0;
  lex_error lerr = E_LEX_OK;
  vector_error verr = E_VECTOR_OK;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);
  ASSERT_EQ(sizeof expected / sizeof expected[0], tokens.index);

  for (i = 0; i < tokens.index; i++) {
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}
