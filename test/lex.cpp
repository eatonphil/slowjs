#include "gtest/gtest.h"

#include "slowjs/lex.h"

TEST(lex, lex) {
  const char *raw_source = "function main() { return a + 1; }";
  vector_char source;
  vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));

  vector_token tokens;
  lex_error err = lex(source, &tokens);

  ASSERT_EQ(E_LEX_OK, err);
  ASSERT_EQ(11, tokens.index);

  const char *expected[] = {
      "function", "main", "(", ")", "{", "return", "a", "+", "1", ";", "}",
  };

  int i;
  for (i = 0; i < tokens.index; i++) {
    vector_char_push(&tokens.elements[i].string, 0);
    ASSERT_STREQ(expected[i], tokens.elements[i].string.elements);
  }
}
