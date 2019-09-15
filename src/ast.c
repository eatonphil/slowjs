#include "slowjs/ast.h"

#include "slowjs/lex.h"

void function_call_free(function_call *fc) {
  int i = 0;

  expression_free(fc->function);
  free(fc->function);

  for (i = 0; i < fc->arguments->index; i++) {
    expression_free(&fc->arguments->elements[i]);
    free(fc->arguments);
  }
}

void op_free(op *o) {
  expression_free(o->left_operand);
  free(o->left_operand);
  expression_free(o->right_operand);
  free(o->right_operand);
}

void expression_free(expression *e) {
  switch (e->type) {
  case EXPRESSION_CALL:
    function_call_free(&e->expression.function_call);
    break;
  case EXPRESSION_OP:
    op_free(&e->expression.op);
    break;
  case EXPRESSION_IDENTIFIER:
    vector_char_free(&e->expression.identifier);
    break;
  case EXPRESSION_BOOL:
  case EXPRESSION_NULL:
  case EXPRESSION_NUMBER:
    break;
  }
}

void variable_declaration_free(variable_declaration *vd) {
  vector_char_free(&vd->name);
  expression_free(&vd->initializer);
}

void statement_free(statement *s) {
  switch (s->type) {
  case STATEMENT_EXPRESSION:
    expression_free(&s->statement.expression);
    break;
  case STATEMENT_RETURN:
    expression_free(&s->statement.ret);
    break;
  case STATEMENT_DECLARATION:
    declaration_free(s->statement.declaration);
    free(s->statement.declaration);
    break;
  }
}

void function_declaration_free(function_declaration *fd) {
  int i = 0;

  vector_char_free(&fd->name);
  vector_string_free(&fd->parameters);

  for (i = 0; i < fd->body.index; i++) {
    statement_free(&fd->body.elements[i]);
  }
}

void declaration_free(declaration *d) {
  int i = 0;

  switch (d->type) {
  case DECLARATION_FUNCTION:
    function_declaration_free(&d->declaration.function);
    break;
  default:
    for (i = 0; i < d->declaration.variable_list.index; i++) {
      variable_declaration_free(&d->declaration.variable_list.elements[i]);
    }
  }
}

void ast_free(ast *a) {
  int i = 0;

  for (i = 0; i < a->declarations.index; i++) {
    declaration_free(&a->declarations.elements[i]);
  }
}
