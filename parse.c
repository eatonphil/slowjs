#include "lex.c"

typedef enum {E_PARSE_OK} parse_error;

// TODO: turn all arrays into vectors
typedef struct {
  char* name;
  vector_expression arguments;
  int arguments_count;
} function_call;

typedef enum {OPERATOR_PLUS} operator_type;

typedef struct {
  operator_type type;
  expression left_operand;
  expression right_operand;
} operator;

typedef enum {EXPRESSION_CALL, EXPRESSION_OPERATOR} expression_type;

typedef struct {
  expression_type type;
  union {
    function_call call;
    operator operator;
  }
} expression;

typedef struct {
  char* name;
  expression initializer;
} variable_declaration;

typedef struct {
  char* name;
  char** parameters;
  int parameters_count;
  expression* body;
} function_declaration;

typedef enum {DECLARATION_FUNCTION, DECLARATION_VAR, DECLARATION_CONST, DECLARATION_LET} declaration_type;

typedef struct {
  declaration_type type;
  union {
    function_declaration function;
    struct {
      variable_declaration* variable_list;
      int variable_list_count;
    } variable;
  };
} declaration;

typedef struct {
  declaration* declarations;
  int declarations_count;
} ast;

parse_error parse(vector_char source, ast* out) {
  parse_error error = E_PARSE_OK;

  ast* program = malloc(sizeof(ast));

  vector_token tokens;
  error = vector_token_init(&tokens);
  if (error != E_VECTOR_OK) {
    LOG_ERROR("vector", "Error during initialization", error);
    goto cleanup_init;
  }

  error = lex(source, &tokens);
  if (error != E_LEX_OK) {
    LOG_ERROR("lex", "Error during initialization", error);
    goto cleanup_lex;
  }

  function_declaration fd;
  bool ok;
  while (true) {
    if (parse_function_declaration(&tokens)) {
      program->declarations
    }
  }

 cleanup_lex:
 cleanup_init:
  vector_token_free(tokens);
  return error;
}
