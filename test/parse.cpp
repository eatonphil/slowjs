#include <stdio.h>

#include "gtest/gtest.h"

extern "C" {
#include "slowjs/lex.h"
#include "slowjs/parse.h"
}

TEST(parse, identifier_bad) {
  vector_token tokens = {};
  vector_char source = {}, test_identifier = {};
  const char raw_source[] = "1";
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_FALSE(parse_identifier(&tokens, &test_identifier));
  vector_token_free(&tokens);
}

TEST(parse, identifier_good) {
  vector_token tokens = {};
  vector_char source = {}, test_identifier = {};
  const char *raw_source[] = {
      "a", "ab", "abcd124", "_", "$", "$12",
  };
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;
  uint64_t i = 0;

  ASSERT_EQ(6, sizeof(raw_source) / sizeof(raw_source[0]));

  for (i = 0; i < (sizeof(raw_source) / sizeof(raw_source[0])); i++) {
    printf("Testing: `%s`\n", raw_source[i]);
    verr =
        vector_char_copy(&source, (char *)raw_source[i], strlen(raw_source[i]));
    ASSERT_EQ(E_VECTOR_OK, verr);

    lerr = lex(source, &tokens);
    ASSERT_EQ(E_LEX_OK, lerr);

    ASSERT_TRUE(parse_identifier(&tokens, &test_identifier));

    vector_token_free(&tokens);
    vector_char_free(&source);
  }
}

TEST(parse, number_good) {
  vector_token tokens = {};
  vector_char source = {};
  const char raw_source[] = "1";
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;
  double test_number = 0;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_TRUE(parse_number(&tokens, &test_number));

  vector_token_free(&tokens);
  vector_char_free(&source);
}

TEST(parse, number_bad) {
  vector_token tokens = {};
  vector_char source = {};
  const char raw_source[] = "b";
  vector_error verr = E_VECTOR_OK;
  lex_error lerr = E_LEX_OK;
  double test_number = 0;

  verr = vector_char_copy(&source, (char *)raw_source, sizeof(raw_source));
  ASSERT_EQ(E_VECTOR_OK, verr);

  lerr = lex(source, &tokens);
  ASSERT_EQ(E_LEX_OK, lerr);

  ASSERT_FALSE(parse_number(&tokens, &test_number));

  vector_token_free(&tokens);
  vector_char_free(&source);
}

TEST(parse, function_good) {
  vector_token tokens = {};
  vector_char source = {};
  struct test {
    const char *src;
    uint expected_tokens;
  } tests[] = {
      {"function a() {}", 6},
      {"function foobar(a) {}", 7},
      {"function b(a, c) {}", 9},
      {"function t() { return 1; }", 9},
  };
  vector_error verr = E_VECTOR_OK;
  function_declaration test_fd = {};
  lex_error lerr = E_LEX_OK;
  uint64_t i = 0;

  ASSERT_EQ(4, sizeof(tests) / sizeof(tests[0]));

  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    printf("Testing: `%s`\n", tests[i].src);

    verr =
        vector_char_copy(&source, (char *)tests[i].src, strlen(tests[i].src));
    ASSERT_EQ(E_VECTOR_OK, verr);

    lerr = lex(source, &tokens);
    ASSERT_EQ(E_LEX_OK, lerr);
    ASSERT_EQ(tests[i].expected_tokens, tokens.index);

    ASSERT_TRUE(parse_function_declaration(&tokens, &test_fd));

    function_declaration_free(&test_fd);
    vector_token_free(&tokens);
    vector_char_free(&source);
  }
}

TEST(parse, statement_good) {
  vector_token tokens = {};
  vector_char source = {};
  struct test {
    const char *src;
    uint expected_tokens;
  } tests[] = {
      {"return 1", 2},
  };
  vector_error verr = E_VECTOR_OK;
  statement test_statement = {};
  lex_error lerr = E_LEX_OK;
  uint64_t i = 0;

  ASSERT_EQ(1, sizeof(tests) / sizeof(tests[0]));

  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    printf("Testing: `%s`\n", tests[i].src);

    verr =
        vector_char_copy(&source, (char *)tests[i].src, strlen(tests[i].src));
    ASSERT_EQ(E_VECTOR_OK, verr);

    lerr = lex(source, &tokens);
    ASSERT_EQ(E_LEX_OK, lerr);
    ASSERT_EQ(tests[i].expected_tokens, tokens.index);

    ASSERT_TRUE(parse_statement(&tokens, &test_statement));

    statement_free(&test_statement);
    vector_token_free(&tokens);
    vector_char_free(&source);
  }
}
