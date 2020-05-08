#include "gtest/gtest.h"

#include "slowjs/lex.h"

TEST(lex, ok) {
  const std::string source = "function a() {return a + b;}";
  std::vector<std::string> expected = {"function", "a", "(", ")", "{", "return", "a", "+", "b", ";", "}"};

  std::vector<Token> tokens = {};
  Error err = lex(source, tokens);

  ASSERT_EQ(err, NoError) << err.error;
  ASSERT_EQ(expected.size(), tokens.size());

  for (unsigned int i = 0; i < expected.size(); i++) {
    ASSERT_EQ(expected[i], tokens[i].value);
  }
}
