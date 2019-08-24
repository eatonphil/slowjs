#ifndef ast
#define ast

#include <string.h>

#include "slowjs/vector.h"

enum parse_error { E_PARSE_OK };
typedef enum parse_error parse_error;

struct expression;
struct vector_expression;

struct function_call {
  struct expression function;
  struct vector_expression arguments;
};
typedef struct function_call function_call;

void function_call_free(function_call *);

enum operator_type {
  OPERATOR_PLUS,
  OPERATOR_MINUS,
  OPERATOR_TIMES,
  OPERATOR_DIV
};
typedef enum operator_type operator_type;

struct operator{
  operator_type type;
  struct expression left_operand;
  struct expression right_operand;
};
typedef struct operator operator;

void operator_free(operator*);

enum expression_type {
  EXPRESSION_CALL,
  EXPRESSION_OPERATOR,
  EXPRESSION_IDENTIFIER,
  EXPRESSION_NUMBER
};
typedef enum expression_type expression_type;

struct expression {
  expression_type type;
  union {
    function_call function_call;
    operator operator;
    double number;
    vector_char identifier;
  } expression;
};
typedef struct expression expression;

DECLARE_VECTOR(expression)

void expression_free(expression *);

struct variable_declaration {
  vector_char name;
  expression initializer;
};
typedef struct variable_declaration variable_declaration;

DECLARE_VECTOR(variable_declaration)

void variable_declaration_free(variable_declaration *);

enum statement_type {
  STATEMENT_EXPRESSION,
  STATEMENT_RETURN,
  STATEMENT_DECLARATION
};
typedef enum statement_type statement_type;

struct statement {
  statement_type type;
  union {
    expression expression;
    expression ret;
    declaration *declaration;
  } statement;
};
typedef struct statement statement;

DECLARE_VECTOR(statement)

void statement_free(statement *);

typedef vector_char string;
DECLARE_VECTOR(string)

struct function_declaration {
  vector_char name;
  vector_string parameters;
  vector_statement body;
};
typedef struct function_declaration function_declaration;

void function_declaration_free(function_declaration *);

enum declaration_type {
  DECLARATION_FUNCTION,
  DECLARATION_VAR,
  DECLARATION_CONST,
  DECLARATION_LET
};
typedef enum declaration_type declaration_type;

struct declaration {
  declaration_type type;
  union {
    function_declaration function;
    vector_variable_declaration variable_list;
  } declaration;
};
typedef struct declaration declaration;

DECLARE_VECTOR(declaration)

void declaration_free(declaration *);

struct ast {
  vector_declaration declarations;
};
typedef struct ast ast;

void ast_free(ast *);

#endif
