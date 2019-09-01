#include "slowjs/file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "slowjs/common.h"

file_error read_file(char *path, vector_char *source_out) {
  void *contents = 0;
  int fd = 0;
  off_t len = 0;
  file_error ferr = E_FILE_OK;
  vector_error verr = E_VECTOR_OK;

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    LOG_ERROR("libc", "Error opening file", errno);
    ferr = E_FILE_OPEN;
    goto cleanup_open;
  }

  len = lseek(fd, 0, SEEK_END);
  contents = mmap(0, len + 1, PROT_READ, MAP_PRIVATE, fd, 0);
  if (!contents) {
    LOG_ERROR("libc", "Error reading file", errno);
    ferr = E_FILE_READ;
    goto cleanup_read;
  }

  verr = vector_char_copy(source_out, (char *)contents, len);
  if (verr != E_VECTOR_OK) {
    LOG_ERROR("vector", "Error copying file", verr);
    ferr = E_FILE_READ;
    goto cleanup_read;
  }

  munmap(contents, len);

cleanup_read:
  if (close(fd) != 0) {
    LOG_ERROR("libc", "Error closing file", errno);
    ferr = E_FILE_CLOSE;
  }
cleanup_open:
  return ferr;
}
