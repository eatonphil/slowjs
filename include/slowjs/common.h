#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#define LOG_ERROR(triggering_module, msg, e)                                   \
  fprintf(stderr, "[%s:%d] %s from %s: %d\n", __FILE__, __LINE__, msg,         \
          triggering_module, e)

#endif
