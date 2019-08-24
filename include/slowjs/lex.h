#ifndef _LEX_H_
#define _LEX_H_

#include "slowjs/vector.h"

enum lex_error { E_LEX_OK, E_LEX_INIT };
typedef enum lex_error lex_error;

struct token {
  int line;
  int col;
  vector_char string;
};
typedef struct token token;

DECLARE_VECTOR(token)

lex_error lex(vector_char, vector_token *);

#endif
