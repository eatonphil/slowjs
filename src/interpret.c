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
  vector_char name;
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
  vector_context child_ctx = {0};
  value function = {0}, v = {0};
  vector_char p = {0};
  context mapping = {0};
  uint64_t i = 0;
  interpret_error err = E_INTERPRET_OK;

  err = interpret_expression(*fc.function, ctx, &function);
  if (err != E_INTERPRET_OK) {
    return err;
  }

  if (function.type != VALUE_CLOSURE) {
    return E_INTERPRET_CALL_NONFUNCTION;
  }

  if (vector_context_copy(&child_ctx, ctx->elements, ctx->index) !=
      E_VECTOR_OK) {
    // TODO: log that main is not a function
    return E_INTERPRET_CRASH;
  }

  for (i = 0; i < function.value.closure.parameters.index; i++) {
    // TODO: guard against inequal number of arguments
    p = function.value.closure.parameters.elements[i];
    err = interpret_expression(fc.arguments->elements[i], ctx, &v);
    if (err != E_INTERPRET_OK) {
      return err;
    }

    mapping.name = p;
    mapping.value = v;
    vector_context_push(&child_ctx, mapping);
  }

  return interpret_statements(function.value.closure.body, &child_ctx, result);
}

interpret_error interpret_op(op o, vector_context *ctx, value *result) {
  value left = {0}, right = {0};
  interpret_error err = E_INTERPRET_OK;

  err = interpret_expression(*o.left_operand, ctx, &left);
  if (err != E_INTERPRET_OK) {
    return err;
  }

  err = interpret_expression(*o.right_operand, ctx, &right);
  if (err != E_INTERPRET_OK) {
    return err;
  }

  result->type = VALUE_NUMBER;
  switch (o.type) {
  case OP_PLUS:
    result->value.number = left.value.number + right.value.number;
    break;
  case OP_MINUS:
    result->value.number = left.value.number - right.value.number;
    break;
  case OP_TIMES:
    result->value.number = left.value.number * right.value.number;
    break;
  case OP_DIV:
    result->value.number = left.value.number / right.value.number;
  }

  return E_INTERPRET_OK;
}

interpret_error interpret_expression(expression e, vector_context *ctx,
                                     value *result) {
  uint64_t i = 0;
  bool matched = false;

  switch (e.type) {
  case EXPRESSION_IDENTIFIER:
    for (i = 0; i < ctx->index; i++) {
      matched = strncmp(ctx->elements[i].name.elements,
                        e.expression.identifier.elements,
                        ctx->elements[i].name.index) == 0;
      if (matched) {
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
  case EXPRESSION_OP:
    return interpret_op(e.expression.op, ctx, result);
  }
}

interpret_error interpret_statement(statement s, vector_context *ctx,
                                    value *result) {
  value nothing = {0};

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
  uint64_t i = 0;
  interpret_error err = E_INTERPRET_OK;

  for (i = 0; i < body.index; i++) {
    err = interpret_statement(body.elements[i], ctx, result);
    if (err != E_INTERPRET_OK) {
      return err;
    }
  }

  return E_INTERPRET_OK;
}

interpret_error interpret_function_declaration(function_declaration fd,
                                               vector_context *ctx) {
  closure c = {0};
  value v = {0};
  context mapping = {0};

  c.ctx = ctx;
  c.parameters = fd.parameters;
  c.body = fd.body;

  v.value.closure = c;
  v.type = VALUE_CLOSURE;

  mapping.name = fd.name;
  mapping.value = v;

  return (interpret_error)vector_context_push(ctx, mapping);
}

interpret_error interpret_declaration(declaration d, vector_context *ctx) {
  if (d.type == DECLARATION_FUNCTION) {
    return interpret_function_declaration(d.declaration.function, ctx);
  }

  // return interpret_variable_list(d.variable_list, ctx);
  return E_INTERPRET_CRASH;
}

interpret_error call_main(vector_context *ctx, function_declaration *main) {
  expression function = {0};
  vector_expression arguments = {0};
  value v = {0};
  function_call fc = {0};
  interpret_error err = E_INTERPRET_OK;

  function.type = EXPRESSION_IDENTIFIER;
  vector_char_copy(&function.expression.identifier, "main", 4);

  fc.function = &function;
  fc.arguments = &arguments;

  err = interpret_function_call(fc, ctx, &v);
  if (err != E_INTERPRET_OK) {
    return err;
  }

  printf("%lf\n", v.value.number);
  return E_INTERPRET_OK;
}

interpret_error interpret(ast program) {
  vector_context ctx = {0};
  declaration d = {0};
  function_declaration main = {0};
  uint64_t i = 0;
  interpret_error err = E_INTERPRET_OK;
  bool found_main = false, matched = false;

  for (i = 0; i < program.declarations.index; i++) {
    d = program.declarations.elements[i];

    err = interpret_declaration(d, &ctx);
    if (err != E_INTERPRET_OK) {
      return err;
    }

    matched = strncmp(d.declaration.function.name.elements, "main", 4) == 0;
    if (d.type == DECLARATION_FUNCTION && matched) {
      main = d.declaration.function;
      found_main = true;
    }
  }

  if (!found_main) {
    LOG_ERROR("interpret", "Expected main function", 0);
    return E_INTERPRET_NO_MAIN;
  }

  return call_main(&ctx, &main);
}
