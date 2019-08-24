#ifndef lex
#define lex

#include "slowjs/vector.h"

typedef enum { E_LEX_OK, E_LEX_INIT } lex_error;

typedef struct {
  int line;
  int col;
  vector_char string;
} token;

DECLARE_VECTOR(token);

lex_error lex(vector_char, vector_token *);

#endif
