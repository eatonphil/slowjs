#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "slowjs/file.h"
#include "slowjs/interpret.h"
#include "slowjs/lex.h"
#include "slowjs/parse.h"
#include "slowjs/vector.h"

// SOURCE: https://stackoverflow.com/a/77336/1507139
void generateBacktrace(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char **argv) {
  struct sigaction action = {0};
  action.sa_handler = generateBacktrace;
  if (sigaction(SIGSEGV, &action, NULL) < 0) {
    printf("TNET: Unable to register segfault handler.\n");
    return 1;
  }

  if (argc != 2) {
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
