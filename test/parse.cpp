#include "gtest/gtest.h"

extern "C" {
#include "slowjs/lex.h"
#include "slowjs/parse.h"
}

TEST(parse, addition) {
  vector_token tokens = {0};
  vector_char source = {0}, test_identifier = {0};
  const char raw_source[] = "1";
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_NE(parse_identifier(tokens, &test_identifier), 0);
}
