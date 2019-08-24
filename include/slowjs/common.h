#ifndef _COMMON_H_
#define _COMMON_H_

typedef enum { false, true } bool;

#define LOG_ERROR(triggering_module, msg, errno)                               \
  fprintf(stderr, "[%s:%d] %s from %s: %d", __FILE__, __LINE__, msg,           \
          triggering_module, errno)

#endif
