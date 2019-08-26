#include "slowjs/parse.h"

#include <stdio.h>
#include <string.h>

#include "slowjs/common.h"
#include "slowjs/lex.h"

#define PARSE_ERROR(msg, t)                                                    \
  fprintf(stderr, "%s near %d:%d.\n", msg, t.line, t.col)

bool parse_literal(vector_token *tokens, const char *match) {
  token t = {0};
  int error = vector_token_get(tokens, 0, &t);
  if (error != E_VECTOR_OK) {
    return false;
  }

  if (!strcmp(t.string.elements, match)) {
    return false;
  }

  vector_token_shift(tokens);
  return true;
}

bool parse_identifier(vector_token *tokens, vector_char *identifer_out) {
  token t = {0};
  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    return false;
  }

  int i;
  char c;

  for (i = 0; i < t.string.index; i++) {
    c = t.string.elements[i];
    if (c != '$' && c != '_' &&
        !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
      continue;
    }

    if (i == 0 || !(c >= '0' && c <= '9')) {
      return false;
    }
  }

  vector_token_shift(tokens);
  return true;
}

bool parse_parameters(vector_token *tokens, vector_string *parameters) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  int i;
  token t = {0};
  char c;
  vector_char parameter = {0};

  while (true) {
    if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
      goto failed_match;
    }

    if (strcmp(t.string.elements, ")")) {
      break;
    }

    if (parameters->index > 0 && !parse_literal(tokens, ",")) {
      PARSE_ERROR("Expected comma after parameter", t);
      goto failed_match;
    }

    if (!parse_identifier(tokens, &parameter)) {
      PARSE_ERROR("Invalid identifier", t);
      goto failed_match;
    }

    if (vector_string_push(parameters, parameter) != E_VECTOR_OK) {
      goto failed_match;
    }
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_expression(vector_token *, expression *);
bool parse_expressions(vector_token *, vector_expression *);

bool parse_function_call(vector_token *tokens, function_call *fc) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  expression function = {0};
  if (!parse_expression(tokens, &function)) {
    goto failed_match;
  }

  if (!parse_literal(tokens, "(")) {
    goto failed_match;
  }

  vector_expression expressions = {0};
  if (!parse_expressions(tokens, &expressions)) {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_number(vector_token *tokens, double *n) {
  char *notfound;
  token t = {0};

  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    return false;
  }

  // 0 pad the string
  vector_char_push(&t.string, 0);

  *n = strtod(t.string.elements, &notfound);
  if (t.string.elements != notfound) {
    vector_token_shift(tokens);
    return true;
  }

  return false;
}

bool parse_operator(vector_token *tokens, operator*o) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  expression left = {0}, right = {0};
  if (!parse_expression(tokens, &left)) {
    goto failed_match;
  }

  token t = {0};
  if (!(o->left_operand = malloc(sizeof(expression))) ||
      !(o->right_operand = malloc(sizeof(expression)))) {
    goto failed_match;
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
    vector_token_get(tokens, 0, &t);
    PARSE_ERROR("Invalid operator", t);
    goto failed_match;
  }

  if (!parse_expression(tokens, &right)) {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_expression(vector_token *tokens, expression *e) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  vector_char id = {0};
  if (parse_identifier(tokens, &id)) {
    e->type = EXPRESSION_IDENTIFIER;
    e->expression.identifier = id;
    return true;
  }

  double n;
  if (parse_number(tokens, &n)) {
    e->type = EXPRESSION_NUMBER;
    e->expression.number = n;
  }

  function_call fc = {0};
  if (parse_function_call(tokens, &fc)) {
    e->type = EXPRESSION_CALL;
    e->expression.function_call = fc;
    return true;
  }

  operator o = {0};
  if (parse_operator(tokens, &o)) {
    e->type = EXPRESSION_OPERATOR;
    e->expression.operator= o;
    return true;
  }

  if (parse_literal(tokens, "(")) {
    if (parse_expression(tokens, e)) {
      if (parse_literal(tokens, ")")) {
        return true;
      }
    }

    vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  }

  return false;
}

bool parse_expressions(vector_token *tokens, vector_expression *expressions) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  expression e = {0};
  while (true) {
    if (parse_literal(tokens, ")")) {
      break;
    }

    if (expressions->index > 0 && !parse_literal(tokens, ",")) {
      goto failed_match;
    }

    if (!parse_expression(tokens, &e)) {
      goto failed_match;
    }

    if (vector_expression_push(expressions, e) != E_VECTOR_OK) {
      goto failed_match;
    }
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_declaration(vector_token *tokens, declaration *);

bool parse_statement(vector_token *tokens, statement *statement) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  declaration d = {0};
  expression e = {0};

  if (parse_declaration(tokens, &d)) {
    statement->type = STATEMENT_DECLARATION;
    statement->statement.declaration = malloc(sizeof(declaration));
    if (statement->statement.declaration == 0) {
      goto failed_match;
    }

    memcpy(statement->statement.declaration, &d, sizeof(declaration));
  } else if (parse_expression(tokens, &e)) {
    statement->type = STATEMENT_EXPRESSION;
    statement->statement.expression = e;
  } else if (parse_literal(tokens, "return") && parse_expression(tokens, &e)) {
    statement->type = STATEMENT_RETURN;
    statement->statement.ret = e;
  } else {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_statements(vector_token *tokens, vector_statement *statements) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  statement s = {0};

  while (true) {
    if (parse_literal(tokens, "}")) {
      break;
    }

    if (statements->index > 0 && !parse_literal(tokens, ";")) {
      goto failed_match;
    }

    if (!parse_statement(tokens, &s)) {
      goto failed_match;
    }

    if (vector_statement_push(statements, s) != E_VECTOR_OK) {
      goto failed_match;
    }
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
  return false;
}

bool parse_function_declaration(vector_token *tokens,
                                function_declaration *fd) {
  vector_token tokens_original = {0};
  vector_token_copy(&tokens_original, tokens->elements, tokens->index);

  if (!parse_literal(tokens, "function")) {
    goto failed_match;
  }

  token t = {0};
  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    PARSE_ERROR("Expected function name", t);
    goto failed_match;
  }

  if (vector_char_copy(&fd->name, t.string.elements, t.string.index) !=
      E_VECTOR_OK) {
    PARSE_ERROR("Expected function name", t);
    goto failed_match;
  }

  if (!parse_literal(tokens, "(")) {
    PARSE_ERROR("Expected parenthesis after function name", t);
    goto failed_match;
  }

  if (!parse_parameters(tokens, &fd->parameters)) {
    PARSE_ERROR("Expected parameters", t);
    goto failed_match;
  }

  if (!parse_literal(tokens, ")")) {
    PARSE_ERROR("Expected closing parenthesis", t);
    goto failed_match;
  }

  if (!parse_literal(tokens, "{")) {
    PARSE_ERROR("Expected opening brace", t);
    goto failed_match;
  }

  if (!parse_statements(tokens, &fd->body)) {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original.elements, tokens_original.index);
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
  } else if (parse_const_declaration(tokens, &vd)) {
    d->type = DECLARATION_CONST;
    d->declaration.variable_list = vd;
  } else if (parse_let_declaration(tokens, &vd)) {
    d->type = DECLARATION_LET;
    d->declaration.variable_list = vd;
  } else if (parse_var_declaration(tokens, &vd)) {
    d->type = DECLARATION_VAR;
    d->declaration.variable_list = vd;
  } else {
    return false;
  }

  return true;
}

parse_error parse(vector_char source, ast *program_out) {
  parse_error error = E_PARSE_OK;

  vector_token tokens = {0};
  error = (parse_error)lex(source, &tokens);
  if (error != E_LEX_OK) {
    LOG_ERROR("lex", "Error during initialization", error);
    goto cleanup_lex;
  }

  int i;
  for (i = 0; i < tokens.index; i++) {
    vector_char_push(&tokens.elements[i].string, 0);
    printf("%s:%d\n", tokens.elements[i].string.elements,
           tokens.elements[i].string.index - 1);
  }

  token t = {0};
  if (!tokens.index) {
    PARSE_ERROR("Program is empty", t);
    error = E_PARSE_EMPTY;
    goto cleanup_lex;
  }

  declaration d = {0};

  while (tokens.index) {
    d = (declaration){0};
    if (!parse_declaration(&tokens, &d)) {
      PARSE_ERROR("Expected top-level declaration", tokens.elements[0]);
      error = E_PARSE_TOPLEVEL_DECLARATION;
      goto cleanup_lex;
    }
    vector_declaration_push(&program_out->declarations, d);
  }

cleanup_lex:
cleanup_init:
  vector_token_free(&tokens);
  return error;
}
