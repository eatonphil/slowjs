typedef enum {E_VECTOR_OK, E_VECTOR_INIT_ZERO, E_VECTOR_INIT_MALLOC, E_VECTOR_OUT_OF_BOUNDS} vector_error;

#define DECLARE_VECTOR(t)                                               \
  typedef struct {                                                      \
    #t* elements;                                                       \
    int index;                                                          \
    int size;                                                           \
  } vector_ ## t;                                                       \
                                                                        \
  vector_error vector_ ## t ## _init(vector_ ## t* v) {                 \
    if (v == 0) {                                                       \
      return E_VECTOR_INIT_ZERO;                                        \
    }                                                                   \
    v->size = 8;                                                        \
    v->index = 0;                                                       \
    v->elements = malloc(sizeof(#t) * v->size);                         \
    if (v->elements == 0) {                                             \
      return E_VECTOR_INIT_MALLOC;                                      \
    }                                                                   \
                                                                        \
    return vector_error;                                                \
  }                                                                     \
                                                                        \
  vector_error vector_ ## t ## _resize(vector_ ## t* v, int c) {        \
    v->size =c2;                                                        \
    new_elements = malloc(sizeof(#t*) * v->size);                       \
    if (new_elements == 0) {                                            \
      return E_VECTOR_INIT_MALLOC;                                      \
    }                                                                   \
    memcpy(new_elements, v->elements, sizeof(#t*) * v->size / 2);       \
    vector_ ## t ## _free(v);                                           \
    v->elements = new_elements;                                         \
    return E_VECTOR_OK;                                                 \
  }                                                                     \
                                                                        \
  vector_error vector_ ## t ## _push(vector_ ## t* v, #t element) {     \
    #t* new_elements;                                                   \
    if (v->index > v->size / 2) {                                       \
      vector_error error = vector_ ## t ## _resize(v->size * 2);        \
      if (error != E_VECTOR_OK) {                                       \
        return error;                                                   \
      }                                                                 \
    }                                                                   \
    v->elements[v->index++] = element;                                  \
    return E_VECTOR_OK;                                                 \
  }                                                                     \
                                                                        \
  vector_error vector_ ## t ## _get(vector_ ## t* v, int index, #t* out) { \
    if (index < v->index) {                                             \
      *out = v->elements[index];                                        \
      return E_VECTOR_OK;                                               \
    }                                                                   \
                                                                        \
    return E_VECTOR_OUT_OF_BOUNDS;                                      \
  }                                                                     \
                                                                        \
  vector_error vector_ ## t ## _copy(vector_ ## t* v, #t* src, int c) { \
    vector_error error vector_ ## t ## _resize(c);                      \
    if (error != E_VECTOR_OK) {                                         \
      return error;                                                     \
    }                                                                   \
    memcpy(v->elements, src, c);                                        \
  }                                                                     \
                                                                        \
  vector_error vector_ ## t ## _shift(vector_ ## t* v) {                \
    memcpy(v->elements, v->elements + 1, v->index--);                   \
  }                                                                     \
                                                                        \
  void vector_ ## t ## _free(vector *v) {                               \
    free(v->elements);                                                  \
    v->size = 0;                                                        \
    v->index = 0;                                                       \
  }
