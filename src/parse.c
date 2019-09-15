#include "slowjs/parse.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define PARSE_ERROR(msg, t)                                                    \
  fprintf(stderr, "%s near \"%s\" at %llu:%llu.\n", msg, t.string.elements,    \
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

#define SLICE(tokens, slice, t, err)                                           \
  err = vector_token_pop(tokens, &t);                                          \
  if (err != E_VECTOR_OK) {                                                    \
    goto cleanup;                                                              \
  }                                                                            \
                                                                               \
  err = vector_token_push(&slice, t);                                          \
  if (err != E_VECTOR_OK) {                                                    \
    goto cleanup;                                                              \
  }

bool parse_number(vector_token *tokens, double *n) {
  token t = {0};
  char *notfound = 0;
  vector_error err = E_VECTOR_OK;

  err = vector_token_pop(tokens, &t);
  if (err != E_VECTOR_OK) {
    goto cleanup; // EOF?
  }

  errno = 0;
  *n = strtod(t.string.elements, &notfound);
  if (*n == 0 && (errno != 0 || t.string.elements == notfound)) {
    goto cleanup;
  }

  return true;

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

  // TODO: is this going to crash if match is longer?
  matched = strncmp(t.string.elements, match, strlen(match)) == 0;
  if (!matched) {
    goto cleanup;
  }

  return true;

cleanup:
  PUSH(tokens, t, err);
  return false;
}

bool parse_identifier(vector_token *tokens, vector_char *identifer_out) {
  uint64_t i = 0;
  char c = 0;
  token t = {0};
  vector_error err = E_VECTOR_OK;
  uint64_t token_length = 0;

  err = vector_token_pop(tokens, &t);
  if (err == E_VECTOR_POP) {
    goto cleanup;
  }

  token_length = t.string.index - 1; // Omit trailing \0
  for (i = 0; i < token_length; i++) {
    c = t.string.elements[i];

    // Can start with [$_a-Z]
    if (c == '$' || c == '_' ||
        ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
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
  uint64_t i = 0;
  vector_error err = 0;
  char c = 0;
  bool found_closing_paren = false;

  STORE_TOKENS_COPY(tokens, &copy, err);

  while (true) {
    if (parse_literal(tokens, ")")) {
      break;
    }

    if (parameters->index > 0 && !parse_literal(tokens, ",")) {
      vector_token_get(tokens, tokens->index - 1, &t);
      PARSE_ERROR("Expected comma after parameter", t);
      goto cleanup;
    }

    if (!parse_identifier(tokens, &parameter)) {
      vector_token_get(tokens, tokens->index - 1, &t);
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
  vector_token copy = {0}, slice = {0};
  expression function = {0};
  token t = {0};
  vector_error err = E_VECTOR_OK;
  bool matched = false;

  if (tokens->index < 2) {
    return false;
  }

  STORE_TOKENS_COPY(tokens, &copy, err);

  matched =
      strncmp(tokens->elements[tokens->index - 2].string.elements, "(", 1) == 0;
  if (!matched) {
    goto cleanup;
  }

  SLICE(tokens, slice, t, err);

  if (!parse_expression(&slice, &function)) {
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

bool parse_binary_op(vector_token *tokens, op *o) {
  const char *ops[] = {"+", "-", "/", "*"};
  vector_token copy = {0}, slice = {0};
  expression left = {0}, right = {0};
  token t = {0};
  uint64_t i = 0;
  vector_error err = E_VECTOR_OK;
  bool matched = false;

  if (tokens->index < 2) {
    return false;
  }

  t = tokens->elements[tokens->index - 2];
  for (i = 0; i < (sizeof ops / sizeof ops[0]); i++) {
    matched = strncmp(t.string.elements, ops[i], 1) == 0;
    if (!matched) {
      continue;
    }

    switch (ops[i][0]) {
    case '+':
      o->type = OP_PLUS;
      break;
    case '-':
      o->type = OP_MINUS;
      break;
    case '*':
      o->type = OP_TIMES;
      break;
    case '/':
      o->type = OP_DIV;
      break;
    default:
      vector_token_get(tokens, tokens->index - 1, &t);
      PARSE_ERROR("Unhandled op", t);
      goto cleanup;
    }

    break;
  }

  if (!matched) {
    return false;
  }

  STORE_TOKENS_COPY(tokens, &copy, err);

  SLICE(tokens, slice, t, err);
  if (!parse_expression(&slice, &left)) {
    goto cleanup;
  }

  // Drop the op
  err = vector_token_pop(tokens, &t);
  if (err != E_VECTOR_OK) {
    goto cleanup;
  }

  if (!parse_expression(tokens, &right)) {
    goto cleanup;
  }

  o->left_operand = (expression *)malloc(sizeof(expression));
  if (o->left_operand == 0) {
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Out of memory", t);
    goto cleanup;
  }
  memcpy(o->left_operand, &left, sizeof(expression));

  o->right_operand = (expression *)malloc(sizeof(expression));
  if (o->right_operand == 0) {
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Out of memory", t);
    goto cleanup;
  }
  memcpy(o->right_operand, &right, sizeof(expression));

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
  op o = {0};
  double n = 0;
  vector_error err = E_VECTOR_OK;

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

  if (parse_function_call(tokens, &fc)) {
    e->type = EXPRESSION_CALL;
    e->expression.function_call = fc;
    return true;
  }

  if (parse_binary_op(tokens, &o)) {
    e->type = EXPRESSION_OP;
    e->expression.op = o;
    return true;
  }

  if (parse_identifier(tokens, &id)) {
    e->type = EXPRESSION_IDENTIFIER;
    e->expression.identifier = id;
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
  vector_error err = E_VECTOR_OK;

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

  // TODO: support `return;`
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
  token t = {0};
  vector_error err = E_VECTOR_OK;

  STORE_TOKENS_COPY(tokens, &copy, err);

  if (!parse_literal(tokens, "{")) {
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Expected opening brace", t);
    goto cleanup;
  }

  while (true) {
    if (parse_literal(tokens, "}")) {
      break;
    }

    if (!parse_statement(tokens, &s)) {
      vector_token_get(tokens, tokens->index - 1, &t);
      PARSE_ERROR("Expected statement", t);
      goto cleanup;
    }

    if (vector_statement_push(statements, s) != E_VECTOR_OK) {
      goto cleanup;
    }

    if (statements->index > 0 && !parse_literal(tokens, ";")) {
      vector_token_get(tokens, tokens->index - 1, &t);
      PARSE_ERROR("Expected semi-colon", t);
      goto cleanup;
    }
  }

  vector_token_free(&copy);
  return true;

cleanup:
  statement_free(&s);
  RESTORE_TOKENS_COPY(tokens, &copy, err);
  return false;
}

bool parse_function_declaration(vector_token *tokens,
                                function_declaration *fd) {
  vector_token copy = {0};
  token t = {0};
  vector_error err = E_VECTOR_OK;

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
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Expected parenthesis after function name", t);
    goto cleanup;
  }

  if (!parse_parameters(tokens, &fd->parameters)) {
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Expected parameters", t);
    goto cleanup;
  }

  if (!parse_block(tokens, &fd->body)) {
    vector_token_get(tokens, tokens->index - 1, &t);
    PARSE_ERROR("Expected body", t);
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

  vector_variable_declaration_free(&vd);
  function_declaration_free(&fd);
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
