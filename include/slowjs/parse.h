#ifndef parse
#define parse

#include "slowjs/ast.h"
#include "slowjs/vector.h"

typedef enum { E_PARSE_OK } parse_error;

#define PARSE_ERROR(msg, t)                                                    \
  LOG_ERROR("parse", sprintf("%s near %d:%d", msg, t->line, t->col), 0)

parse_error parse(vector_char, ast *);

#endif
