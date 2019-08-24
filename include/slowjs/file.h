#ifndef _FILE_H_
#define _FILE_H_

#include "slowjs/vector.h"

typedef enum { E_FILE_OK, E_FILE_OPEN, E_FILE_CLOSE, E_FILE_READ } file_error;

file_error read_file(char *, vector_char *);

#endif
