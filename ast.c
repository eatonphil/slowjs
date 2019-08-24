#include <string.h>

#include "lex.c"

typedef enum {E_PARSE_OK} parse_error;

// Forward declarations
struct vector_expression;
struct expression;
void expression_free(expression* e);
struct declaration;
void declaration_free(declaration* d);

typedef struct {
  expression function;
  vector_expression arguments;
} function_call;

void function_call_free(function_call* fc) {
  expression_free(&fc->function);
  int i;
  for (i = 0; i < fc->arguments.index; i++) {
    expression_free(&fc->arguments.elements[i]);
  }
}

typedef enum {OPERATOR_PLUS, OPERATOR_MINUS, OPERATOR_TIMES, OPERATOR_DIV} operator_type;

typedef struct {
  operator_type type;
  expression left_operand;
  expression right_operand;
} operator;

void operator_free(operator* o) {
  expression_free(&o->left_operand);
  expression_free(&o->right_operand);
}

typedef enum {EXPRESSION_CALL, EXPRESSION_OPERATOR, EXPRESSION_IDENTIFIER, EXPRESSION_NUMBER} expression_type;

typedef struct {
  expression_type type;
  union {
    function_call function_call;
    operator operator;
    number double;
    vector_char identifier;
  }
} expression;

void expression_free(expression* e) {
  switch (e->type) {
  case EXPRESSION_CALL:
    function_call_free(&e->function_call);
    break;
  case EXPRESSION_OPERATOR:
    operator_free(&e->operator);
    break;
  case EXPRESSION_IDENTIFIER:
    vector_char_free(&e->identifier);
    break;
  }
}

DECLARE_VECTOR(expression);

typedef struct {
  vector_char name;
  expression initializer;
} variable_declaration;

DECLARE_VECTOR(variable_declaration);

void variable_declaration_free(variable_declaration* vd) {
  vector_char_free(&vd->name);
  expression_free(&vd->initializer);
}

typedef enum {STATEMENT_EXPRESSION, STATEMENT_RETURN, STATEMENT_DECLARATION} statement_type;

typedef struct {
  statement_type type;
  union {
    expression expression;
    expression ret;
    declaration declaration;
  }
} statement;

void statement_free(statement* s) {
  switch (s->type) {
  case STATEMENT_EXPRESSION:
    expression_free(&s->expression);
    break;
  case STATEMENT_RETURN:
    expression_free(&s->ret);
    break;
  case STATEMENT_DECLARATION:
    declaration_free(&s->declaration);
    break;
  }
}

DECLARE_VECTOR(statement);

DECLARE_VECTOR(string);

typedef struct {
  vector_char name;
  vector_string parameters;
  vector_statement body;
} function_declaration;

void function_declaration_free(function_declaration* fd) {
  vector_char_free(&fd->name);
  vector_string_free(&fd->parameters);

  int i;
  for (i = 0; i < fd->body.index; i++) {
    statement_free(&fd->body.elements[i]);
  }
}

typedef enum {DECLARATION_FUNCTION, DECLARATION_VAR, DECLARATION_CONST, DECLARATION_LET} declaration_type;

typedef struct {
  declaration_type type;
  union {
    function_declaration function;
    vector_variable_declaration variable_list;
  };
} declaration;

DECLARE_VECTOR(declaration);

void declaration_free(declaration* d) {
  int i;

  switch (d->type) {
  case DECLARATION_FUNCTION:
    function_declaration_free(&d->function);
    break;
  default:
    for (i = 0; i < d->variable_list.index; i++) {
      variable_declaration_free(&d->variable_list.elements[i]);
    }
  }
}

typedef struct {
  vector_declaration declarations;
} ast;

void ast_free(ast* a) {
  int i;
  for (i = 0; i < a->declarations.index; i++) {
    declaration_free(&a->declarations.elements[i]);
  }
}
