#ifndef _PARSE_H_
#define _PARSE_H_

#include "slowjs/ast.h"
#include "slowjs/vector.h"

typedef enum {
  E_PARSE_OK,
  E_PARSE_EMPTY,
  E_PARSE_TOPLEVEL_DECLARATION,
} parse_error;

parse_error parse(vector_char, ast *);

#endif
