#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  E_VECTOR_OK,
  E_VECTOR_INIT_ZERO,
  E_VECTOR_INIT_MALLOC,
  E_VECTOR_OUT_OF_BOUNDS
} vector_error;

#define DECLARE_VECTOR(t)                                                      \
  struct vector_##t {                                                          \
    int index;                                                                 \
    int size;                                                                  \
    t *elements;                                                               \
  };                                                                           \
  typedef struct vector_##t vector_##t;                                        \
                                                                               \
  static vector_error vector_##t##_init(vector_##t *);                         \
  static vector_error vector_##t##_resize(vector_##t *, int);                  \
  static vector_error vector_##t##_push(vector_##t *, t);                      \
  static vector_error vector_##t##_get(vector_##t *v, int, t *);               \
  static vector_error vector_##t##_copy(vector_##t *v, t *, int);              \
  static void vector_##t##_shift(vector_##t *);                                \
  static void vector_##t##_free(vector_##t *);                                 \
                                                                               \
  static vector_error vector_##t##_init(vector_##t *v) {                       \
    if (v == 0) {                                                              \
      return E_VECTOR_INIT_ZERO;                                               \
    }                                                                          \
    v->size = 8;                                                               \
    v->index = 0;                                                              \
    v->elements = malloc(sizeof(t) * v->size);                                 \
    if (v->elements == 0) {                                                    \
      return E_VECTOR_INIT_MALLOC;                                             \
    }                                                                          \
                                                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_resize(vector_##t *v, int c) {              \
    t *new_elements = malloc(sizeof(t) * c);                                   \
    if (new_elements == 0) {                                                   \
      return E_VECTOR_INIT_MALLOC;                                             \
    }                                                                          \
    memcpy(new_elements, v->elements, sizeof(t) * v->size);                    \
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
    vector_error error;                                                        \
    if (v->size != 0) {                                                        \
      error = vector_##t##_init(v);                                            \
                                                                               \
      if (error != E_VECTOR_OK) {                                              \
        return error;                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    if (v->index > v->size / 2) {                                              \
      error = vector_##t##_resize(v, v->size * 2);                             \
      if (error != E_VECTOR_OK) {                                              \
        return error;                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    v->elements[v->index++] = element;                                         \
                                                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_get(vector_##t *v, int index, t *out) {     \
    if (index < v->index) {                                                    \
      *out = v->elements[index];                                               \
      return E_VECTOR_OK;                                                      \
    }                                                                          \
                                                                               \
    return E_VECTOR_OUT_OF_BOUNDS;                                             \
  }                                                                            \
                                                                               \
  static vector_error vector_##t##_copy(vector_##t *v, t *src, int c) {        \
    vector_error error = vector_##t##_resize(v, c);                            \
    if (error != E_VECTOR_OK) {                                                \
      return error;                                                            \
    }                                                                          \
    memcpy(v->elements, src, c);                                               \
    return E_VECTOR_OK;                                                        \
  }                                                                            \
                                                                               \
  static void vector_##t##_shift(vector_##t *v) {                              \
    memcpy(v->elements, v->elements + 1, v->index--);                          \
  }                                                                            \
                                                                               \
  static void vector_##t##_free(vector_##t *v) {                               \
    if (v->size) {                                                             \
      free(v->elements);                                                       \
    }                                                                          \
    v->size = 0;                                                               \
    v->index = 0;                                                              \
  }

DECLARE_VECTOR(char)
typedef vector_char string;
DECLARE_VECTOR(string)

#endif
