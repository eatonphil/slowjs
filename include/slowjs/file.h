#ifndef file
#define file

#include "slowjs/vector.h"

typedef enum { E_FILE_OK, E_FILE_OPEN, E_FILE_CLOSE, E_FILE_READ } file_error;

DECLARE_VECTOR(char)

file_error read_file(char *, vector_char *);

#endif
