#ifndef _AST_H_
#define _AST_H_

#include <string.h>

#include "slowjs/vector.h"

struct declaration;
struct expression;
struct vector_expression;

struct function_call {
  struct expression *function;
  struct vector_expression *arguments;
};
typedef struct function_call function_call;

void function_call_free(function_call *);

enum op_type { OP_PLUS, OP_MINUS, OP_TIMES, OP_DIV };
typedef enum op_type op_type;

struct op {
  op_type type;
  struct expression *left_operand;
  struct expression *right_operand;
};
typedef struct op op;

void op_free(op *);

enum expression_type {
  EXPRESSION_CALL,
  EXPRESSION_OP,
  EXPRESSION_IDENTIFIER,
  EXPRESSION_NUMBER
};
typedef enum expression_type expression_type;

struct expression {
  expression_type type;
  union {
    function_call function_call;
    op op;
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
    struct expression expression;
    struct expression ret;
    struct declaration *declaration;
  } statement;
};
typedef struct statement statement;

DECLARE_VECTOR(statement)

void statement_free(statement *);

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
