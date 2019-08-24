#include "slowjs/lex.h"

vector_error vector_token_push_char(vector_token *v, int line, int col,
                                    char c) {
  token t;
  t.col = col;
  t.line = line;

  error = vector_char_push(&t.source, c);
  if (error != E_VECTOR_OK) {
    return error;
  }

  return vector_token_push(tokens_out, t);
}

lex_error lex(vector_char source, vector_token *tokens_out) {
  lex_error error = E_LEX_OK;

  char c;
  int i;
  bool in_comment = false;

  token current;
  for (i = 0; i < source.index; i++) {
    c = vector_char_get(&source, i);

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
    if (c == '/' && i + 1 < source.index &&
        vector_char_get(&source, i + 1) == '/') {
      in_comment = true;
      continue;
    }

    switch (c) {
    case ' ':
    case '\t':
      if (!current.index) {
        continue;
      }

      error = vector_token_push(tokens_out, current);
      if (error != E_VECTOR_OK) {
        goto cleanup_loop;
      }

      vector_char_free(&current.source);
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
      if (current.index) {
        error = vector_token_push(tokens_out, current);
        if (error != E_VECTOR_OK) {
          goto cleanup_loop;
        }

        vector_char_free(&current->source);
      }

      error = vector_token_push_char(tokens_out, c);
      if (error != E_VECTOR_OK) {
        goto cleanup_loop;
      }

      break;
    default:
      error = vector_char_push(&current.string, c);
      if (error != E_VECTOR_OK) {
        goto cleanup_loop;
      }

      break;
    }
  }

cleanup_loop:
  LOG_ERROR("vector",
            sprintf("Error lexing near %d:%d", current.line, current.col),
            error);
  vector_char_free(&current.string);
cleanup_init:
  return error;
}
