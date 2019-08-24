#include "slowjs/parse.h"

bool parse_literal(vector_token *tokens, string match) {
  token t;
  int error = vector_token_get(tokens, 0, &t);
  if (error != E_VECTOR_OK) {
    return false;
  }

  if (!strcmp((string)t.string->elements, string)) {
    return false;
  }

  return vector_token_shift(tokens) == E_VECTOR_OK;
}

bool parse_identifier(vector_token *tokens, vector_char *identifer_out) {
  token t;
  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    return false;
  }

  for (i = 0; i < t->index; i++) {
    c = t->elements[i];
    if (c != '$' && c != '_' &&
        !((c >= 'a' && c <= 'z') ||)(c >= 'A' && c <= 'Z')) {
      continue;
    }

    if (i == 0 || !(c >= '0' && c <= '9')) {
      return false;
    }
  }

  return vector_token_shift(tokens) == E_VECTOR_OK;
}

bool parse_parameters(vector_token *tokens, vector_string *parameters) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  int i;
  token t;
  char c;
  vector_char parameter;
  while (true) {
    if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
      goto failed_match;
    }

    if (strcmp((string)t->elements, ")")) {
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
  vector_token_copy(tokens, tokens_original, tokens_original->index);
  return false;
}

bool parse_function_call(vector_token *tokens, function_call *fc) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  expression function;
  if (!parse_expression(tokens, &function)) {
    goto failed_match;
  }

  if (!parse_literal(tokens, "(")) {
    goto failed_match;
  }

  vector_expression expressions;
  if (!parse_expressions(tokens, &expressions)) {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original, tokens_original->index);
  return false;
}

bool parse_number(vector_token *tokens, double *n) {
  char *notfound;
  token t;

  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    return false;
  }

  *n = strtod(s, &notfound);
  if (s != notfound) {
    return vector_token_shift(tokens) == E_VECTOR_OK;
  }

  return false;
}

bool parse_operator(vector_token *tokens, expression *e) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  expression left, right;
  if (!parse_expression(tokens, &left)) {
    goto failed_match;
  }

  token t;
  operator_type ot;
  if (parse_literal(tokens, "+")) {
    ot = OPERATOR_PLUS;
  } else if (parse_literal(tokens, "-")) {
    ot = OPERATOR_MINUS;
  } else if (parse_literal(tokens, "*")) {
    ot = OPERATOR_TIMES;
  } else if (parse_literal(tokens, "/")) {
    ot = OPERATOR_DIV;
  } else {
    vector_token_get(tokens, 0, &t);
    PARSE_ERROR("Invalid operator", t);
    goto failed_match;
  }

  if (!parse_expression(tokens, &right)) {
    goto failed_match;
  }

  e->type = ot;
  e->left_operand = left;
  e->right_operand = right;

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original, tokens_original->index);
  return false;
}

bool parse_expression(vector_token *tokens, expression *e) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  vector_char id;
  if (parse_identifier(tokens, &id)) {
    expresion->type = EXPRESSION_IDENTIFIER;
    expression->identifier = id;
    return true;
  }

  double n;
  if (parse_number(tokens, &n)) {
    expression->type = EXPRESSION_NUMBER;
    expression->number = n;
  }

  function_call fc;
  if (parse_function_call(tokens, &fc)) {
    expression->type = EXPRESSION_CALL;
    expression->function_call = fc;
    return true;
  }

  operator o;
  if (parse_operator(tokens, &o)) {
    expression->type = EXPRESSION_OPERATOR;
    expression->operator= o;
    return true;
  }

  if (parse_syntax(tokens, "(")) {
    if (parse_expression(tokens, e)) {
      if (parse_syntax(tokens, ")")) {
        return true;
      }
    }

    vector_token_copy(tokens, tokens_original, tokens_original->index);
  }

  return false;
}

bool parse_expressions(vector_token *tokens, vector_expression *expressions) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  expression e;
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

    if (vector_expression_push(block, e) != E_VECTOR_OK) {
      goto failed_match;
    }
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original, tokens_original->index);
  return false;
}

bool parse_statement(vector_token *tokens, statement *statement) {
  declaration d;

  if (parse_declaration(tokens, &d)) {
  }
}

bool parse_statements(vector_token *tokens, vector_statement *statements) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  statement s;
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

    if (vector_statement_push(block, s) != E_VECTOR_OK) {
      goto failed_match;
    }
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original, tokens_original->index);
  return false;
}

bool parse_function_declaration(vector_token *tokens,
                                function_declaration *fd) {
  vector_token tokens_original;
  vector_token_copy(tokens_original, tokens->elements, tokens->index);

  if (!parse_literal(tokens, "function")) {
    goto failed_match;
  }

  token t;
  if (vector_token_get(tokens, 0, &t) != E_VECTOR_OK) {
    goto failed_match;
  }

  if (vector_char_copy(vd->name, t->elements, t->index) != E_VECTOR_OK) {
    goto failed_match;
  }

  if (!parse_literal(tokens, "(")) {
    goto failed_match;
  }

  if (!parse_parameters(tokens, &fd->parameters)) {
    goto failed_match;
  }

  if (!parse_literal(tokens, ")")) {
    goto failed_match;
  }

  if (!parse_literal(tokens, "{")) {
    goto failed_match;
  }

  if (!parse_statements(tokens, &fd->body)) {
    goto failed_match;
  }

  return true;

failed_match:
  vector_token_copy(tokens, tokens_original, tokens_original->index);
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

parse_error parse(vector_char source, ast *program_out) {
  parse_error error = E_PARSE_OK;

  vector_token tokens;
  error = lex(source, &tokens);
  if (error != E_LEX_OK) {
    LOG_ERROR("lex", "Error during initialization", error);
    goto cleanup_lex;
  }

  declaration d;
  function_declaration fd;
  variable_declarations vd;

  while (true) {
    d = (declaration){0};
    fd = (function_declaration){0};
    vd = (variable_declarations){0};

    if (parse_function_declaration(&tokens, &fd)) {
      d.type = DECLARATION_FUNCTION;
      d.function = fd;
    } else if (parse_const_declarations(&tokens, &vd)) {
      d.type = DECLARATION_CONST;
      d.vd = vd;
    } else if (parse_let_declarations(&tokens, &vd)) {
      d.type = DECLARATION_LET;
      d.vd = vd;
    } else if (parse_var_declarations(&tokens, &vd)) {
      d.type = DECLARATION_VAR;
      d.vd = vd;
    }

    vector_declaration_push(&program_out->declarations, d);
  }

cleanup_lex:
cleanup_init:
  vector_token_free(tokens);
  return error;
}
