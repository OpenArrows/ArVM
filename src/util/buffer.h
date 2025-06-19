#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdlib.h>

#define BUFFER_BLOCK_SIZE 16

#define BUFFER_T(type)                                                         \
  struct {                                                                     \
    type *value;                                                               \
    size_t count;                                                              \
                                                                               \
    size_t _capacity;                                                          \
  }

#define BUFFER_GROW(buffer, grow_count)                                        \
  if (((buffer).count += grow_count) > (buffer)._capacity) {                   \
    (buffer)._capacity =                                                       \
        (1 + ((buffer).count - 1) / BUFFER_BLOCK_SIZE) * BUFFER_BLOCK_SIZE;    \
    (buffer).value =                                                           \
        realloc((buffer).value, (buffer)._capacity * sizeof(*buffer.value));   \
  } else {                                                                     \
  }

#endif /* BUFFER_H */