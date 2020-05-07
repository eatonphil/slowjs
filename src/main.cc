#include <iostream>
#include <vector>

import lex;

int main() {
  std::vector<Token> tokens = {};
  lex("function a() {return aa + b;}", tokens);
  for (unsigned int i = 0; i < tokens.size(); i++) {
    std::cout << tokens[i].value << std::endl;
  }
}
