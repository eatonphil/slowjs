#ifndef SLOWJS_LEX_H
#define SLOWJS_LEX_H

#include <string>
#include <vector>

struct Error {
  std::string error;
};

static Error NoError = {};

enum TokenType {IdentiferToken, KeywordToken, SymbolToken};

struct Location {
  unsigned int line;
  unsigned int col;
  unsigned int index;
};

struct Token {
  std::string value;
  enum TokenType type;
  Location loc;
};

Error lex(const string& source, std::vector<Token>& tokens);

#endif
