#ifndef SLOWJS_LEX_H
#define SLOWJS_LEX_H

#include <cstdint>
#include <string>
#include <vector>

struct Error {
  std::string error;
};

static bool operator==(const Error& lhs, const Error& rhs) {
  return lhs.error.compare(rhs.error) == 0;
}

static bool operator!=(const Error& lhs, const Error& rhs) {
  return !(lhs == rhs);
}

static Error NoError = {};

enum TokenType {IdentifierToken, KeywordToken, SymbolToken};

struct Location {
  uint64_t line;
  uint64_t col;
  uint64_t index;
};

struct Token {
  std::string value;
  enum TokenType type;
  Location loc;
};

Error lex(const std::string& source, std::vector<Token>& tokens);

#endif
