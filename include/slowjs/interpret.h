#ifndef interpret
#define interpret

#include "slowjs/ast.h"

typedef enum {
  E_INTERPRET_OK,
  E_INTERPRET_NO_MAIN,
  E_INTERPRET_CRASH,
  E_INTERPRET_CALL_NONFUNCTION
} interpret_error;

interpret_error interpret(ast program);

#endif
