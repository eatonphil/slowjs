#include "slowjs/interpret.h"

#include <stdio.h>
#include <string.h>

#include "slowjs/ast.h"
#include "slowjs/common.h"
#include "slowjs/vector.h"

struct vector_context;

struct closure {
  struct vector_context *ctx;
  vector_string parameters;
  vector_statement body;
};
typedef struct closure closure;

enum value_type { VALUE_NUMBER, VALUE_CLOSURE };
typedef enum value_type value_type;

struct value {
  value_type type;
  union {
    double number;
    closure closure;
  } value;
};
typedef struct value value;

DECLARE_VECTOR(value)

struct context {
  vector_char id;
  value value;
};
typedef struct context context;

DECLARE_VECTOR(context)

interpret_error interpret_expression(expression, vector_context *, value *);
interpret_error interpret_statements(vector_statement, vector_context *,
                                     value *);
interpret_error interpret_declaration(declaration, vector_context *);

interpret_error interpret_function_call(function_call fc, vector_context *ctx,
                                        value *result) {
  value function;
  interpret_error error = interpret_expression(*fc.function, ctx, &function);
  if (error != E_INTERPRET_OK) {
    return E_INTERPRET_CRASH;
  }

  if (function.type != VALUE_CLOSURE) {
    return E_INTERPRET_CALL_NONFUNCTION;
  }

  vector_context child_ctx;
  if (vector_context_copy(&child_ctx, ctx->elements, ctx->index)) {
    return E_INTERPRET_CRASH;
  }

  int i;
  value v;
  vector_char p;
  context mapping;
  for (i = 0; i < function.value.closure.parameters.index; i++) {
    // TODO: guard against inequal number of arguments
    p = function.value.closure.parameters.elements[i];
    error = interpret_expression(fc.arguments->elements[i], ctx, &v);
    if (error != E_INTERPRET_OK) {
      return error;
    }

    mapping.id = p;
    mapping.value = v;
    vector_context_push(&child_ctx, mapping);
  }

  return interpret_statements(function.value.closure.body, &child_ctx, result);
}

interpret_error interpret_operator(operator o, vector_context *ctx,
                                   value *result) {
  value left, right;
  interpret_error error = interpret_expression(*o.left_operand, ctx, &left);
  if (error != E_INTERPRET_OK) {
    return error;
  }

  error = interpret_expression(*o.right_operand, ctx, &right);
  if (error != E_INTERPRET_OK) {
    return error;
  }

  result->type = VALUE_NUMBER;
  switch (o.type) {
  case OPERATOR_PLUS:
    result->value.number = left.value.number + right.value.number;
    break;
  case OPERATOR_MINUS:
    result->value.number = left.value.number - right.value.number;
    break;
  case OPERATOR_TIMES:
    result->value.number = left.value.number * right.value.number;
    break;
  case OPERATOR_DIV:
    result->value.number = left.value.number / right.value.number;
  }

  return E_INTERPRET_OK;
}

interpret_error interpret_expression(expression e, vector_context *ctx,
                                     value *result) {
  int i;
  switch (e.type) {
  case EXPRESSION_IDENTIFIER:
    for (i = 0; i < ctx->index; i++) {
      if (strncmp(ctx->elements[i].id.elements,
                  e.expression.identifier.elements,
                  ctx->elements[i].id.index)) {
        *result = ctx->elements[i].value;
        return E_INTERPRET_OK;
      }
    }

    // TODO: handle semantics correctly and usefully
    return E_INTERPRET_CRASH;
  case EXPRESSION_NUMBER:
    result->type = VALUE_NUMBER;
    result->value.number = e.expression.number;
    return E_INTERPRET_OK;
  case EXPRESSION_CALL:
    return interpret_function_call(e.expression.function_call, ctx, result);
  case EXPRESSION_OPERATOR:
    return interpret_operator(e.expression.operator, ctx, result);
  }
}

interpret_error interpret_statement(statement s, vector_context *ctx,
                                    value *result) {
  value nothing;
  switch (s.type) {
  case STATEMENT_RETURN:
    return interpret_expression(s.statement.ret, ctx, result);
  case STATEMENT_EXPRESSION:
    return interpret_expression(s.statement.expression, ctx, &nothing);
  case STATEMENT_DECLARATION:
    return interpret_declaration(*s.statement.declaration, ctx);
  }
}

interpret_error interpret_statements(vector_statement body, vector_context *ctx,
                                     value *result) {
  int i;
  interpret_error error;
  for (i = 0; i < body.index; i++) {
    error = interpret_statement(body.elements[i], ctx, result);
    if (error != E_INTERPRET_OK) {
      return error;
    }
  }

  return E_INTERPRET_OK;
}

interpret_error interpret_function_declaration(function_declaration fd,
                                               vector_context *ctx) {
  closure c = {ctx, fd.parameters, fd.body};
  value v = {VALUE_CLOSURE};
  v.value.closure = c;
  context mapping = {fd.name, v};
  return (interpret_error)vector_context_push(ctx, mapping);
}

interpret_error interpret_declaration(declaration d, vector_context *ctx) {
  if (d.type == DECLARATION_FUNCTION) {
    return interpret_function_declaration(d.declaration.function, ctx);
  }

  // return interpret_variable_list(d.variable_list, ctx);
  return E_INTERPRET_CRASH;
}

interpret_error interpret(ast program) {
  interpret_error error;
  int i;
  declaration d;
  function_declaration main;
  bool found_main = false;

  vector_context ctx;

  for (i = 0; i < program.declarations.index; i++) {
    d = program.declarations.elements[i];

    error = interpret_declaration(d, &ctx);
    if (error != E_INTERPRET_OK) {
      return error;
    }

    if (d.type == DECLARATION_FUNCTION &&
        strcmp(d.declaration.function.name.elements, "main")) {
      main = d.declaration.function;
      found_main = true;
    }
  }

  if (!found_main) {
    LOG_ERROR("interpret", "Expected main function", 0);
    return E_INTERPRET_NO_MAIN;
  }

  expression function;
  function.type = EXPRESSION_IDENTIFIER;
  vector_char_copy(&function.expression.identifier, "main", 4);
  vector_expression arguments;
  function_call fc = {&function, &arguments};
  value v;
  error = interpret_function_call(fc, &ctx, &v);
  if (error != E_INTERPRET_OK) {
    return error;
  }
  printf("%lf\n", v.value.number);

  return E_INTERPRET_OK;
}
