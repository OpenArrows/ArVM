#ifndef ARVM_STACK_H
#define ARVM_STACK_H

#include <limits.h>
#include <stdlib.h>

// Stack

#define ARVM_STACK(t)                                                          \
  struct {                                                                     \
    t *item;                                                                   \
    t *base;                                                                   \
  }

#define ARVM_ST_INIT(st, size)                                                 \
  (((st)->item = (st)->base = malloc(sizeof(*(st)->base) * (size))) != NULL)

#define ARVM_ST_FREE(st) free((st)->base)

#define ARVM_ST_PUSH(st, val) (*((st)->item++) = (val))

#define ARVM_ST_POP(st) (*(--(st)->item))

#endif /* ARVM_STACK_H */