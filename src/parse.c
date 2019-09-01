#include "slowjs/parse.h"

#include <stdio.h>
#include <string.h>

#include "slowjs/common.h"
#include "slowjs/lex.h"

#define PARSE_ERROR(msg, t)                                                    \
  fprintf(stderr, "%s near \"%s\" at %d:%d.\n", msg, t.string.elements,        \
          t.line, t.col - t.string.index)

#define PUSH(tokens, t, err)                                                   \
  err = vector_token_push(tokens, t);                                          \
  if (err != E_VECTOR_OK) {                                                    \
    PARSE_ERROR("Failed to restore token", t);                                 \
    return false;                                                              \
  }

#define STORE_TOKENS_COPY(tokens, copy, err)                                   \
  err = vector_token_copy(copy, (tokens)->elements, (tokens)->index);          \
  if (err != E_VECTOR_OK) {                                                    \
    LOG_ERROR("parse", "Failed to copy tokens", 0);                            \
    return false;                                                              \
  }

#define RESTORE_TOKENS_COPY(tokens, copy, err)                                 \
  STORE_TOKENS_COPY(copy, tokens, err)                                         \
  vector_token_free(copy);

bool parse_declaration(vector_token *, declaration *);
bool parse_number(vector_token *, double *);
bool parse_literal(vector_token *, const char *);
bool parse_identifier(vector_token *, vector_char *);
bool parse_parameters(vector_token *, vector_string *);
bool parse_function_call(vector_token *, function_call *);
bool parse_operator(vector_token *, operator*);
bool parse_expression(vector_token *, expression *);
bool parse_expressions(vector_token *, vector_expression *);
bool parse_statement(vector_token *, statement *);
bool parse_block(vector_token *, vector_statement *);
bool parse_function_declaration(vector_token *, function_declaration *);
bool parse_const_declaration(vector_token *, vector_variable_declaration *);
bool parse_let_declaration(vector_token *, vector_variable_declaration *);
bool parse_var_declaration(vector_token *, vector_variable_declaration *);
bool parse_declaration(vector_token *, declaration *);

bool parse_number(vector_token *tokens, double *n) {
  token t = {0};
  token *tp = 0;
  char *notfound = 0;
  int err = E_PARSE_OK;

  err = vector_token_pop(tokens, tp);
  if (tp == 0) {
    goto cleanup; // EOF?
  }

  t = *tp;

  *n = strtod(t.string.elements, &notfound);
  if (t.string.elements != notfound) {
    return true;
  }

cleanup:
  PUSH(tokens, t, err);
  return false;
}

bool parse_literal(vector_token *tokens, const char *match) {
  token t = {0};
  vector_error err = E_VECTOR_OK;
  bool matched = false;

  err = vector_token_pop(tokens, &t);
  if (err == E_VECTOR_POP) {
    goto cleanup;
  }

  matched = strncmp(t.string.elements, match, t.string.index) == 0;
  if (!matched) {
    goto cleanup;
  }

  return true;

cleanup:
  PUSH(tokens, t, err);
  return false;
}

bool parse_identifier(vector_token *tokens, vector_char *identifer_out) {
  int i = 0;
  char c = 0;
  token t = {0};
  vector_error err = vector_token_pop(tokens, &t);
  if (err == E_VECTOR_POP) {
    goto cleanup;
  }

  for (i = 0; i < t.string.index; i++) {
    c = t.string.elements[i];

    // Can start with [$_a-Z]
    if (c != '$' && c != '_' &&
        !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
      continue;
    }

    if (i > 0 && c >= '0' && c <= '9') {
      continue;
    }

    goto cleanup;
  }

  return true;

cleanup:
  PUSH(tokens, t, err);
  return false;
}

bool parse_parameters(vector_token *tokens, vector_string *parameters) {
  vector_char parameter = {0};
  vector_token copy = {0};
  token t = {0};
  int i = 0, err = 0;
  char c = 0;
  bool matched = false;

  STORE_TOKENS_COPY(tokens, &copy, err);

  while (true) {
    err = vector_token_pop(tokens, &t);
    if (err != E_VECTOR_OK) {
      goto cleanup;
    }

    matched = strncmp(t.string.elements, ")", 1) == 0;
    if (matched) {
      break;
    }

    if (parameters->index > 0 && !parse_literal(tokens, ",")) {
      PARSE_ERROR("Expected comma after parameter", t);
      goto cleanup;
    }

    if (!parse_identifier(tokens, &parameter)) {
      PARSE_ERROR("Invalid identifier", t);
      goto cleanup;
    }

    if (vector_string_push(parameters, parameter) != E_VECTOR_OK) {
      goto cleanup;
    }
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_function_call(vector_token *tokens, function_call *fc) {
  vector_expression expressions = {0};
  vector_token copy = {0};
  expression function = {0};
  int err = E_PARSE_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  if (!parse_expression(tokens, &function)) {
    goto cleanup;
  }

  if (!parse_literal(tokens, "(")) {
    goto cleanup;
  }

  if (!parse_expressions(tokens, &expressions)) {
    goto cleanup;
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_operator(vector_token *tokens, operator*o) {
  vector_token copy = {0};
  expression left = {0}, right = {0};
  token t = {0};
  int err = E_PARSE_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  if (!parse_expression(tokens, &left)) {
    goto cleanup;
  }

  if (!(o->left_operand = malloc(sizeof(expression))) ||
      !(o->right_operand = malloc(sizeof(expression)))) {
    goto cleanup;
  }
  memcpy(o->left_operand, &left, sizeof(expression));
  memcpy(o->right_operand, &right, sizeof(expression));

  if (parse_literal(tokens, "+")) {
    o->type = OPERATOR_PLUS;
  } else if (parse_literal(tokens, "-")) {
    o->type = OPERATOR_MINUS;
  } else if (parse_literal(tokens, "*")) {
    o->type = OPERATOR_TIMES;
  } else if (parse_literal(tokens, "/")) {
    o->type = OPERATOR_DIV;
  } else {
    vector_token_pop(tokens, &t);
    PARSE_ERROR("Invalid operator", t);
    goto cleanup;
  }

  if (!parse_expression(tokens, &right)) {
    goto cleanup;
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_expression(vector_token *tokens, expression *e) {
  vector_token copy = {0};
  vector_char id = {0};
  function_call fc = {0};
  operator o = {0};
  double n = 0;
  vector_error err = E_VECTOR_OK;

  printf("here? ofoofoo\n");
  if (parse_literal(tokens, "(")) {
    STORE_TOKENS_COPY(tokens, &copy, err);

    if (parse_expression(tokens, e)) {
      if (parse_literal(tokens, ")")) {
        return true;
      }
    }

    RESTORE_TOKENS_COPY(tokens, &copy, err);
    return false;
  }

  printf("here? ofoofoo\n");
  if (parse_identifier(tokens, &id)) {
    printf("here? ofoofoo222\n");
    e->type = EXPRESSION_IDENTIFIER;
    e->expression.identifier = id;
    return true;
  }

  printf("here? ofoofoo\n");
  if (parse_function_call(tokens, &fc)) {
    e->type = EXPRESSION_CALL;
    e->expression.function_call = fc;
    return true;
  }

  printf("here? ofoofoo\n");
  if (parse_operator(tokens, &o)) {
    e->type = EXPRESSION_OPERATOR;
    e->expression.operator= o;
    return true;
  }

  if (parse_number(tokens, &n)) {
    e->type = EXPRESSION_NUMBER;
    e->expression.number = n;
    return true;
  }

  return false;
}

bool parse_expressions(vector_token *tokens, vector_expression *expressions) {
  vector_token copy = {0};
  expression e = {0};
  int err = E_PARSE_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  while (true) {
    if (parse_literal(tokens, ")")) {
      break;
    }

    if (expressions->index > 0 && !parse_literal(tokens, ",")) {
      goto cleanup;
    }

    if (!parse_expression(tokens, &e)) {
      goto cleanup;
    }

    if (vector_expression_push(expressions, e) != E_VECTOR_OK) {
      goto cleanup;
    }
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_statement(vector_token *tokens, statement *statement) {
  declaration d = {0};
  expression e = {0};

  if (parse_declaration(tokens, &d)) {
    statement->type = STATEMENT_DECLARATION;
    statement->statement.declaration = malloc(sizeof(declaration));
    if (statement->statement.declaration == 0) {
      return false;
    }

    memcpy(statement->statement.declaration, &d, sizeof(declaration));
    return true;
  }

  if (parse_literal(tokens, "return") && parse_expression(tokens, &e)) {
    statement->type = STATEMENT_RETURN;
    statement->statement.ret = e;
    return true;
  }

  if (parse_expression(tokens, &e)) {
    statement->type = STATEMENT_EXPRESSION;
    statement->statement.expression = e;
    return true;
  }

  return false;
}

bool parse_block(vector_token *tokens, vector_statement *statements) {
  vector_token copy = {0};
  statement s = {0};
  int err = E_PARSE_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  printf("here now?\n");
  while (true) {
    if (parse_literal(tokens, "}")) {
      break;
    }

    if (statements->index > 0 && !parse_literal(tokens, ";")) {
      goto cleanup;
    }

    printf("here now now?\n");
    if (!parse_statement(tokens, &s)) {
      goto cleanup;
    }

    if (vector_statement_push(statements, s) != E_VECTOR_OK) {
      goto cleanup;
    }
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_function_declaration(vector_token *tokens,
                                function_declaration *fd) {
  vector_token copy = {0};
  token t = {0};
  int err = E_PARSE_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  if (!parse_literal(tokens, "function")) {
    goto cleanup;
  }

  if (vector_token_pop(tokens, &t) != E_VECTOR_OK) {
    PARSE_ERROR("Expected function name", t);
    goto cleanup;
  }

  if (vector_char_copy(&fd->name, t.string.elements, t.string.index) !=
      E_VECTOR_OK) {
    PARSE_ERROR("Expected function name", t);
    goto cleanup;
  }

  if (!parse_literal(tokens, "(")) {
    PARSE_ERROR("Expected parenthesis after function name", t);
    goto cleanup;
  }

  if (!parse_parameters(tokens, &fd->parameters)) {
    PARSE_ERROR("Expected parameters", t);
    goto cleanup;
  }

  if (!parse_literal(tokens, "{")) {
    PARSE_ERROR("Expected opening brace", t);
    goto cleanup;
  }

  if (!parse_block(tokens, &fd->body)) {
    goto cleanup;
  }

  vector_token_free(&copy);
  return true;

cleanup:
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_const_declaration(vector_token *tokens,
                             vector_variable_declaration *vd) {
  if (!parse_literal(tokens, "const")) {
    return false;
  }

  return false;
}

bool parse_let_declaration(vector_token *tokens,
                           vector_variable_declaration *vd) {
  if (!parse_literal(tokens, "let")) {
    return false;
  }

  return false;
}

bool parse_var_declaration(vector_token *tokens,
                           vector_variable_declaration *vd) {
  if (!parse_literal(tokens, "var")) {
    return false;
  }

  return false;
}

bool parse_declaration(vector_token *tokens, declaration *d) {
  vector_variable_declaration vd = {0};
  function_declaration fd = {0};

  if (parse_function_declaration(tokens, &fd)) {
    d->type = DECLARATION_FUNCTION;
    d->declaration.function = fd;
    return true;
  }

  if (parse_const_declaration(tokens, &vd)) {
    d->type = DECLARATION_CONST;
    d->declaration.variable_list = vd;
    return true;
  }

  if (parse_let_declaration(tokens, &vd)) {
    d->type = DECLARATION_LET;
    d->declaration.variable_list = vd;
    return true;
  }

  if (parse_var_declaration(tokens, &vd)) {
    d->type = DECLARATION_VAR;
    d->declaration.variable_list = vd;
    return true;
  }

  return false;
}

parse_error parse(vector_char source, ast *program_out) {
  vector_token tokens = {0};
  declaration d = {0};
  parse_error err = E_PARSE_OK;

  tokens.element_free = token_element_free;
  err = (parse_error)lex(source, &tokens);
  if (err != E_LEX_OK) {
    LOG_ERROR("lex", "Error during initialization", err);
    goto cleanup;
  }

  if (!tokens.index) {
    LOG_ERROR("parse", "Program is empty", 0);
    err = E_PARSE_EMPTY;
    goto cleanup;
  }

  while (tokens.index) {
    d = (declaration){0};
    if (!parse_declaration(&tokens, &d)) {
      LOG_ERROR("parse", "Expected top-level declaration", 0);
      err = E_PARSE_TOPLEVEL_DECLARATION;
      goto cleanup;
    }

    err = (parse_error)vector_declaration_push(&program_out->declarations, d);
    if (err != E_VECTOR_OK) {
      LOG_ERROR("parse", "Failed storing declaration", 0);
      goto cleanup;
    }
  }

cleanup:
  vector_token_free(&tokens);
  return err;
}
