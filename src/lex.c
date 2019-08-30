#include "slowjs/lex.h"

#include <stdio.h>

#include "slowjs/common.h"

#define LEX_ERROR(msg, t) fprintf(stderr, "%s near %d:%d\n", msg, t.line, t.col)

vector_error vector_token_push_char(vector_token *v, int line, int col,
                                    char c) {
  token t = {0};
  t.col = col;
  t.line = line;

  vector_error err = vector_char_copy(&t.string, &c, 1);
  if (err != E_VECTOR_OK) {
    return err;
  }

  return vector_token_push(v, t);
}

lex_error lex(vector_char source, vector_token *tokens_out) {
  lex_error err = E_LEX_OK;

  char c;
  int i;
  bool in_comment = false;

  token current = {0};

  for (i = 0; i < source.index; i++) {
    c = source.elements[i];

    if (c == '\n') {
      in_comment = false;
      current.line++;
      continue;
    } else {
      current.col++;
      if (in_comment) {
        continue;
      }
    }

    // Handle line comments
    if (c == '/' && i + 1 < source.index && source.elements[i + 1] == '/') {
      in_comment = true;
      continue;
    }

    switch (c) {
    case ' ':
    case '\t':
      if (!current.string.index) {
        continue;
      }

      err = (lex_error)vector_token_push(tokens_out, current);
      if (err != E_VECTOR_OK) {
        LEX_ERROR("Error lexing", current);
        goto cleanup_loop;
      }

      current = (token){0};
      break;
    case '{':
    case '}':
    case '(':
    case ')':
    case '[':
    case ']':
    case ';':
    case '+':
    case '-':
    case '/':
    case '*':
    case '&':
    case ',':
    case '.':
    case ':':
      // TODO: support double character operators: +=, ++, &&, etc.
      if (current.string.index) {
        err = (lex_error)vector_token_push(tokens_out, current);
        if (err != E_VECTOR_OK) {
          LEX_ERROR("Error lexing", current);
        }

        current = (token){0};
      }

      err = (lex_error)vector_token_push_char(tokens_out, current.line,
                                              current.col, c);
      if (err != E_VECTOR_OK) {
        LEX_ERROR("Error lexing", current);
        goto cleanup_loop;
      }

      break;
    default:
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9')) {
        err = (lex_error)vector_char_push(&current.string, c);
        if (err != E_VECTOR_OK) {
          LEX_ERROR("Error lexing", current);
          goto cleanup_loop;
        }
      }

      break;
    }
  }

  if (current.string.index) {
    err = (lex_error)vector_token_push(tokens_out, current);
    if (err != E_VECTOR_OK) {
      LEX_ERROR("Error lexing", current);
    }
  }

cleanup_loop:
cleanup_init:
  return err;
}
