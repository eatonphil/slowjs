typedef enum { false, true } bool;

#define LOG_ERROR(triggering_module, msg, errno)                        \
  fprintf(stderr, "[%s:%d] %s from %s: %d", __FILE__, __LINE__, msg, triggering_module, errno)

#include "vector.c"
#include "file.c"
#include "parse.c"
#include "interpret.c"

int main(int argc, char **argv) {
  int error = 0;

  if (argc !== 2) {
    printf("Expected a JavaScript file argument, got nothing.");
    return 1;
  }

  vector_char source;
  int error = read_file(argv[1], &source);
  if (error != E_FILE_OK) {
    error = error;
    goto cleanup_file;
  }

  ast program;
  error = parse(source, &program);
  if (error != E_PARSE_OK) {
    error = error;
    goto cleanup_parse;
  }

  error = interpret(program);
  if (error != E_INTERPRET_OK) {
    error = error;
    goto cleanup_interpret;
  }

 cleanup_interpret:
  ast_free(&program);
 cleanup_parse:
  vector_char_free(&source);
 cleanup_file:
  return error;
}
