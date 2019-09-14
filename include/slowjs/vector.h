#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) a < b ? a : b

typedef enum {
  E_VECTOR_OK,
  E_VECTOR_INIT_ZERO,
  E_VECTOR_INIT_MALLOC,
  E_VECTOR_OUT_OF_BOUNDS,
  E_VECTOR_TOO_BIG,
  E_VECTOR_POP,
} vector_error;

#define DECLARE_VECTOR(t)                                                      \
  struct vector_##t {                                                          \
    uint64_t index;                                                            \
    uint64_t size;                                                             \
    t *elements;                                                               \
    void (*element_free)(t *);                                                 \
  };                                                                           \
  typedef struct vector_##t vector_##t;                                        \
                                                                               \
  static vector_error vector_##t##_init(vector_##t *);                         \
  static vector_error vector_##t##_resize(vector_##t *, uint64_t);             \
  static vector_error vector_##t##_push(vector_##t *, t);                      \
  static vector_error vector_##t##_pop(vector_##t *, t *);                     \
  static vector_error vector_##t##_get(vector_##t *, uint64_t, t *);           \
  static vector_error vector_##t##_set(vector_##t *, uint64_t, t *);           \
  static vector_error vector_##t##_copy(vector_##t *, t *, uint64_t);          \
  static void vector_##t##_free(vector_##t *);                                 \
                                                                               \
  static vector_error vector_##t##_init(vector_##t *v) {                       \
    if (v == 0) {                                                              \
      return E_VECTOR_INIT_ZERO;                                               \
    }                                                                          \
    v->size = 8;                                                               \
    v->index = 0;                                                              \
    v->elements = (t *)malloc(sizeof(t) * v->size);                            \
    if (v->elements == 0) {                                                    \
      return E_VECTOR_INIT_MALLOC;                                             \
    }                                                                          \
                                                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_resize(vector_##t *v, uint64_t c) {         \
    t *new_elements = 0;                                                       \
                                                                               \
    new_elements = (t *)malloc(sizeof(t) * c);                                 \
    if (new_elements == 0) {                                                   \
      return E_VECTOR_INIT_MALLOC;                                             \
    }                                                                          \
                                                                               \
    if (c < v->index) {                                                        \
      memcpy(new_elements, v->elements, sizeof(t) * c);                        \
    } else if (v->index) {                                                     \
      memcpy(new_elements, v->elements, sizeof(t) * (v->index));               \
    }                                                                          \
                                                                               \
    if (v->size) {                                                             \
      free(v->elements);                                                       \
    }                                                                          \
                                                                               \
    v->elements = new_elements;                                                \
    v->size = c;                                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_push(vector_##t *v, t element) {            \
    uint64_t newsize = 0;                                                      \
    vector_error err = E_VECTOR_OK;                                            \
                                                                               \
    if (v->size == 0) {                                                        \
      err = vector_##t##_init(v);                                              \
                                                                               \
      if (err != E_VECTOR_OK) {                                                \
        return err;                                                            \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (v->index > v->size / 2) {                                              \
      newsize = MIN(v->size * 2, (uint64_t)(-1));                              \
      if (newsize == v->size) {                                                \
        return E_VECTOR_TOO_BIG;                                               \
      }                                                                        \
                                                                               \
      err = vector_##t##_resize(v, newsize);                                   \
      if (err != E_VECTOR_OK) {                                                \
        return err;                                                            \
      }                                                                        \
    }                                                                          \
                                                                               \
    v->elements[v->index++] = element;                                         \
                                                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_pop(vector_##t *v, t *element) {            \
    if (v->index > 0) {                                                        \
      --v->index;                                                              \
      *element = v->elements[v->index];                                        \
      return E_VECTOR_OK;                                                      \
    }                                                                          \
                                                                               \
    return E_VECTOR_POP;                                                       \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_get(vector_##t *v, uint64_t index,          \
                                       t *out) {                               \
    if (index < v->index) {                                                    \
      *out = v->elements[index];                                               \
      return E_VECTOR_OK;                                                      \
    }                                                                          \
                                                                               \
    return E_VECTOR_OUT_OF_BOUNDS;                                             \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_set(vector_##t *v, uint64_t index, t *in) { \
    if (index < v->index) {                                                    \
      memcpy(v->elements + index, in, sizeof(t));                              \
      return E_VECTOR_OK;                                                      \
    }                                                                          \
                                                                               \
    return E_VECTOR_OUT_OF_BOUNDS;                                             \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_copy(vector_##t *v, t *src, uint64_t c) {   \
    vector_error err = E_VECTOR_OK;                                            \
                                                                               \
    err = vector_##t##_resize(v, c);                                           \
    if (err != E_VECTOR_OK) {                                                  \
      return err;                                                              \
    }                                                                          \
    memcpy(v->elements, src, sizeof(t) * c);                                   \
    v->index = c;                                                              \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static void vector_##t##_free(vector_##t *v) {                               \
    uint64_t i;                                                                \
    if (v->element_free) {                                                     \
      for (i = 0; i < v->index; i++) {                                         \
        v->element_free(&v->elements[i]);                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (v->size) {                                                             \
      free(v->elements);                                                       \
    }                                                                          \
                                                                               \
    v->size = 0;                                                               \
    v->index = 0;                                                              \
  }

DECLARE_VECTOR(char)
typedef vector_char string;
DECLARE_VECTOR(string)

#endif
