#include <slowjs/lex.h>

#include <iostream>
#include <algorithm>

std::string longestMatch(const std::string& source, const uint64_t index, const std::vector<std::string>& options) {
  std::vector<bool> skipList(options.size());

  uint64_t matchIndex = 0;
  std::string match;
  while (matchIndex < source.length() &&
	 std::count(skipList.begin(), skipList.end(), false) > 0) {
    for (uint64_t i = 0; i < options.size(); i++) {
      if (skipList[i]) {
	continue;
      }

      if (matchIndex > options[i].length() ||
	  source.substr(index, matchIndex).compare(options[i].substr(0, matchIndex)) != 0) {
	skipList[i] = true;
	continue;
      }

      if (matchIndex == options[i].length() &&
	  source.substr(index, matchIndex).compare(options[i]) == 0 &&
	  matchIndex > match.length()) {
	match = options[i];
      }

      matchIndex++;
    }
  }

  return match;
}

typedef std::tuple<Error, Location, Token> lexResult;

lexResult lexKeyword(const std::string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc, Token{}};
  }

  const std::vector<std::string> keywords = {"function", "return"};
  const std::string match = longestMatch(source, loc.index, keywords);
  if (match.size() == 0) {
    return {NoError, loc, Token{}};
  }

  return {
	  NoError,
	  Location{loc.line, loc.col+match.size(), loc.index+match.size()},
	  Token{match, KeywordToken, loc},
  };
}

lexResult lexSymbol(const std::string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc, Token{}};
  }

  switch (source[loc.index]) {
  case ' ':
  case '\t':
  case '\n':
    return {
	    NoError,
	    {
	     loc.line + (source[loc.index] == '\n' ? 1 : 0),
	     loc.col + (source[loc.index] == '\n' ? 0 : 1),
	     loc.index + 1,
	    },
	    Token{},
    };
  }

  const std::vector<std::string> keywords = {"{", "}", "(", ")", ";", "+"};
  const std::string match = longestMatch(source, loc.index, keywords);
  if (match.size() == 0) {
    return {NoError, loc, Token{}};
  }

  return {
	  NoError,
	  Location{loc.line, loc.col+match.size(), loc.index+match.size()},
	  Token{match, SymbolToken, loc},
  };
}

lexResult lexIdentifier(const std::string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc, Token{}};
  }

  uint64_t i = loc.index;
  for (; i < source.size(); i++) {
    bool isAlpha = (source[i] >= 'a' && source[i] <= 'z') ||
      (source[i] >= 'A' && source[i] <= 'Z');

    if (isAlpha) {
      continue;
    }

    bool isNumeric = source[i] >= '0' && source[i] <= '9';
    if (i > loc.index && (isNumeric || source[i] == '$')) {
      continue;
    }

    break;
  }

  if (i == loc.index) {
    return {NoError, loc, Token{}};
  }

  return {
	  NoError,
	  {loc.line, loc.col + i - loc.index, i},
	  Token{
		source.substr(loc.index, i - loc.index),
		IdentifierToken,
		loc,
	  },
  };
}

Error lex(const std::string& source, std::vector<Token>& tokens) {
  Location loc = {};

  std::vector<lexResult (*)(const std::string&, const Location)> lexers = {lexKeyword, lexSymbol, lexIdentifier};

  while (loc.index < source.length()) {
    std::cout << loc.index << std::endl;
    for (uint64_t i = 0; i < (sizeof lexers / sizeof lexers[0]); i++) {
      auto [error, newLoc, token] = lexers[i](source, loc);
      if (error != NoError) {
	return error;
      }

      if (newLoc.index > loc.index) {
	loc = newLoc;
	if (token.value != "") {
	  tokens.push_back(token);
	  std::cout << "\"" << token.value << "\"" << std::endl;
	}
	break;
      }

      if (i == (sizeof lexers / sizeof lexers[0] - 1)) {
	return Error{"Unable to lex token", loc};	
      }
    }
  }

  return NoError;
}
