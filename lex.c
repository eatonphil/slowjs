typedef enum {E_LEX_OK, E_LEX_INIT} lex_error;

typedef struct {
  int line;
  int col;
  vector_char string;
} token;

DECLARE_VECTOR(token);

vector_error reset_current(token* current) {
  vector_char_free(&current->source);
  return vector_char_init(&current->source);
}

vector_error vector_token_push_char(vector_token* v, int line, int col, char c) {
  token t;
  t.col = col;
  t.line = line;

  error = vector_char_init(&t.source);
  if (error != E_VECTOR_OK) {
    return error;
  }

  error = vector_char_push(&t.source, c);
  if (error != E_VECTOR_OK) {
    return error;
  }

  return vector_token_push(tokens_out, t);
}

lex_error lex(vector_char source, vector_token* tokens_out) {
  lex_error error = E_LEX_OK;

  char c;
  int i;
  bool in_comment = false;

  token current;
  error = vector_char_init(&current.string);
  if (error != E_VECTOR_OK) {
    goto cleanup_init;
  }

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
    if (c == '/' && i + 1 < source.index && vector_char_get(&source, i + 1) == '/') {
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
      error = vector_char_init(&current.source);
      if (error != E_VECTOR_OK) {
        goto cleanup_loop;
      }

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

        error = reset_current(&current);
        if (error != E_VECTOR_OK) {
          goto cleanup_loop;
        }
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
  LOG_ERROR("vector", sprintf("Error lexing near %d:%d", current.line, current.col), error);
  vector_char_free(&current.string);
 cleanup_init:
  return error;
}
