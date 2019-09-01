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

void generate_backtrace(int);
void register_backtraces();

// SOURCE: https://stackoverflow.com/a/77336/1507139
void generate_backtrace(int sig) {
  void *array[10] = {0};
  size_t size = 0;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void register_backtraces() {
  struct sigaction action = {0};

  action.sa_handler = generate_backtrace;
  if (sigaction(SIGSEGV, &action, NULL) < 0) {
    printf("Unable to register segfault handler.\n");
    exit(1);
  }
}

int main(int argc, char **argv) {
  ast program = {0};
  vector_char source = {0};
  int err = 0;

  register_backtraces();

  if (argc != 2) {
    printf("Expected a JavaScript file argument, got nothing.");
    return 1;
  }

  err = read_file(argv[1], &source);
  if (err != E_FILE_OK) {
    goto cleanup_file;
  }

  err = parse(source, &program);
  if (err != E_PARSE_OK) {
    goto cleanup_parse;
  }

  err = interpret(program);
  if (err != E_INTERPRET_OK) {
    printf("Error interpreting program.\n");
    goto cleanup_interpret;
  }

cleanup_interpret:
  ast_free(&program);
cleanup_parse:
  vector_char_free(&source);
cleanup_file:
  return err;
}
