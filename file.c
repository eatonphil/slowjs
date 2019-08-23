#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum {E_FILE_OK, E_FILE_OPEN, E_FILE_CLOSE, E_FILE_READ} file_error;

DECLARE_VECTOR(char);

file_error read_file(char* path, vector_char* source_out) {
  file_error error = E_FILE_OK;
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    LOG_ERROR("libc", sprintf("Error opening file \"%s\"", path), errno);
    error = E_FILE_OPEN;
    goto cleanup_open;
  }

  off_t len = lseek(fd, 0, SEEK_END);
  void* contents = mmap(0, len + 1, PROT_READ, MAP_PRIVATE, fd, 0);
  if (!contents) {
    LOG_ERROR("libc", sprintf("Error reading file \"%s\"", path), errno);
    error = E_FILE_READ;
    goto cleanup_read;
  }

  vector_error verr = vector_char_copy(source_out, (char*)contents, len);
  if (verr != E_VECTOR_OK) {
    LOG_ERROR("vector", sprintf("Error copying file \"%s\"", path), verr);
    error = E_FILE_READ;
    goto cleanup_read;
  }

  munmap(contents, len);

 cleanup_read:
  if (!close(fd)) {
    LOG_ERROR("libc", sprintf("Error closing file \"%s\"", path), errno);
    error = E_FILE_CLOSE;
  }
 cleanup_open:
  return error;
}
