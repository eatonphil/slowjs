#include "slowjs/ast.h"

void function_call_free(function_call *fc) {
  expression_free(&fc->function);
  int i;
  for (i = 0; i < fc->arguments.index; i++) {
    expression_free(&fc->arguments.elements[i]);
  }
}

void operator_free(operator*o) {
  expression_free(&o->left_operand);
  expression_free(&o->right_operand);
}

void expression_free(expression *e) {
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

void variable_declaration_free(variable_declaration *vd) {
  vector_char_free(&vd->name);
  expression_free(&vd->initializer);
}

void statement_free(statement *s) {
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

void function_declaration_free(function_declaration *fd) {
  vector_char_free(&fd->name);
  vector_string_free(&fd->parameters);

  int i;
  for (i = 0; i < fd->body.index; i++) {
    statement_free(&fd->body.elements[i]);
  }
}

void declaration_free(declaration *d) {
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

void ast_free(ast *a) {
  int i;
  for (i = 0; i < a->declarations.index; i++) {
    declaration_free(&a->declarations.elements[i]);
  }
}
