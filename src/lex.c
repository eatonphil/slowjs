#include "slowjs/lex.h"

#include <stdio.h>

#include "slowjs/common.h"

#define LEX_ERROR(msg, t) fprintf(stderr, "%s near %d:%d\n", msg, t.line, t.col)

vector_error vector_token_push_char(vector_token *, int, int, char);
lex_error reverse_tokens_and_null_terminate(vector_token *);

vector_error vector_token_push_char(vector_token *v, int line, int col,
                                    char c) {
  token t = {0};
  vector_error err = E_VECTOR_OK;

  t.col = col;
  t.line = line;

  err = vector_char_copy(&t.string, &c, 1);
  if (err != E_VECTOR_OK) {
    return err;
  }

  return vector_token_push(v, t);
}

lex_error reverse_tokens_and_null_terminate(vector_token *tokens_out) {
  vector_token copy = {0};
  int i = 0;
  lex_error err = E_LEX_OK;

  err = (lex_error)vector_token_copy(&copy, tokens_out->elements,
                                     tokens_out->index);
  if (err != E_VECTOR_OK) {
    LEX_ERROR("Error copying for reversal", tokens_out->elements[0]);
    return err;
  }

  for (i = 0; i < copy.index; i++) {
    // Null-terminate all tokens
    err = (lex_error)vector_char_push(&copy.elements[i].string, 0);
    if (err != E_VECTOR_OK) {
      LEX_ERROR("Error null-terminating tokens", copy.elements[i]);
      break;
    }

    // Reverse the tokens to form a stack.
    memcpy(&tokens_out->elements[copy.index - i - 1], &copy.elements[i],
           sizeof(token));
  }

  vector_token_free(&copy);
  return err;
}

lex_error lex(vector_char source, vector_token *tokens_out) {
  token current = {0};
  lex_error err = E_LEX_OK;
  char c = 0;
  int i = 0;
  bool in_comment = false;

  for (i = 0; i < source.index; i++) {
    c = source.elements[i];

    if (c == '\n') {
      in_comment = false;
      current.line++;
      continue;
    } else if (c == 0) {
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
    case '\r':
    case '\t':
      if (!current.string.index) {
        continue;
      }

      err = (lex_error)vector_token_push(tokens_out, current);
      if (err != E_VECTOR_OK) {
        LEX_ERROR("Error lexing", current);
        return err;
      }

      current.string = (vector_char){0};
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

        current.string = (vector_char){0};
      }

      err = (lex_error)vector_token_push_char(tokens_out, current.line,
                                              current.col, c);
      if (err != E_VECTOR_OK) {
        LEX_ERROR("Error lexing", current);
        return err;
      }

      current.string = (vector_char){0};

      break;
    default:
      err = (lex_error)vector_char_push(&current.string, c);
      if (err != E_VECTOR_OK) {
        LEX_ERROR("Error lexing", current);
        return err;
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

  return reverse_tokens_and_null_terminate(tokens_out);
}
