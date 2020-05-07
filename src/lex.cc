#include <slowjs/lex.h>

#include <array>
#include <algorithm>

bool operator==(const Error& lhs, const Error& rhs) {
  return lhs.error.compare(rhs.error) == 0;
}

bool operator!=(const Error& lhs, const Error& rhs) {
  return !(lhs == rhs);
}

std::string longestMatch(const string& source, const unsigned int index, const std::array& options) {
  std::vector<bool> skipList(options.size());

  unsigned int matchIndex = 0;
  std::string match;
  while (matchIndex < source.length() &&
	 std::count(skipList.begin(), skipList.end(), false) > 0) {
    for (unsigned int i = 0; i < options.length(); i++) {
      if (skipList[i]) {
	continue;
      }

      if (matchIndex >= options[i].length() ||
	  source.substr(index, matchIndex).compare(options[i].substr(0, matchIndex)) != 0) {
	skipList[i] = true;
	continue;
      }

      if (matchIndex == options[i].length() - 1 &&
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

lexResult lexKeyword(const string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc.index, Token{}};
  }

  const std::array<string> keywords = {"function", "return"};
  const string match = longestMatch(source, loc.index, keywords);
  return {
	  NoError,
	  Location{loc.line, loc.col+match.size(), loc.index+match.size()},
	  Token{match, KeywordType, loc},
  };
}

lexResult lexSymbol(const string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc.index, Token{}};
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

  const std::array<string> keywords = {"{", "}", "(", ")", ";", "+"};
  const string match = longestMatch(source, loc.index, keywords);
  return {
	  NoError,
	  Location{loc.line, loc.col+match.size(), loc.index+match.size()},
	  Token{match, SymbolType, loc},
  };
}

lexResult lexIdentifier(const string& source, const Location loc) {
  if (loc.index >= source.length()) {
    return {NoError, loc, Token{}};
  }

  unsigned int i = loc.index;
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
	  {loc.line, loc.col + i - loc.index, loc.index},
	  Token{
		source.substr(loc.index, i),
		IdentiferType,
		loc,
	  },
  };
}

Error lex(const string& source, std::vector<Token>& tokens) {
  Location loc = {};
  
  std::array<lexResult (*)(const string&, const unsigned int)> lexers = {lexKeyword, lexSymbol, lexIdentifier};

  while (index < source.length()) {
    for (unsigned int i = 0; i < (sizeof lexResult / sizeof lexResult[0]); i++) {
      auto [error, newIndex, token] = lexers[i](source, index);
      if (error != NoError) {
	return error;
      }

      if (newIndex > index) {
	index = newIndex;
	tokens.push_back(token);
	break;
      }
    }
  }

  return NoError;
}
