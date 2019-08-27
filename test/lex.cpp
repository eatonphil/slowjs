#include "gtest/gtest.h"

extern "C" {
#include "slowjs/lex.h"
}

TEST(lex, addition) {
  const char *raw_source = "a + 1";
  vector_char source = {0};
  vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));

  vector_token tokens = {0};
  lex_error err = lex(source, &tokens);

  ASSERT_EQ(E_LEX_OK, err);

  const char *expected[] = {
      "a",
      "+",
      "1",
  };

  ASSERT_EQ(sizeof expected / sizeof expected[0], tokens.index + 1);

  int i;
  for (i = 0; i < tokens.index; i++) {
    vector_char_push(&tokens.elements[i].string, 0);
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}

TEST(lex, function) {
  const char *raw_source = "function main() { return a + 1; }";
  vector_char source = {0};
  vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));

  vector_token tokens = {0};
  lex_error err = lex(source, &tokens);

  ASSERT_EQ(E_LEX_OK, err);

  const char *expected[] = {
      "function", "main", "(", ")", "{", "return", "a", "+", "1", ";", "}",
  };

  ASSERT_EQ(sizeof expected / sizeof expected[0], tokens.index + 1);

  int i;
  for (i = 0; i < tokens.index; i++) {
    vector_char_push(&tokens.elements[i].string, 0);
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}
