#ifndef _PARSE_H_
#define _PARSE_H_

#include "slowjs/ast.h"
#include "slowjs/common.h"
#include "slowjs/lex.h"
#include "slowjs/vector.h"

typedef enum {
  E_PARSE_OK,
  E_PARSE_EMPTY,
  E_PARSE_TOPLEVEL_DECLARATION,
} parse_error;

bool parse_declaration(vector_token *, declaration *);
bool parse_number(vector_token *, double *);
bool parse_literal(vector_token *, const char *);
bool parse_identifier(vector_token *, vector_char *);
bool parse_parameters(vector_token *, vector_string *);
bool parse_function_call(vector_token *, function_call *);
bool parse_binary_op(vector_token *, op *);
bool parse_expression(vector_token *, expression *);
bool parse_expressions(vector_token *, vector_expression *);
bool parse_statement(vector_token *, statement *);
bool parse_block(vector_token *, vector_statement *);
bool parse_function_declaration(vector_token *, function_declaration *);
bool parse_const_declaration(vector_token *, vector_variable_declaration *);
bool parse_let_declaration(vector_token *, vector_variable_declaration *);
bool parse_var_declaration(vector_token *, vector_variable_declaration *);
bool parse_declaration(vector_token *, declaration *);
parse_error parse(vector_char, ast *);

#endif
