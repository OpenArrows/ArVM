#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct arvm_arena_block arvm_arena_block_t;

struct arvm_arena_block {
  arvm_arena_block_t *next;
  size_t allocated;
  size_t capacity;
};

typedef struct arvm_arena {
  size_t block_size;

  arvm_arena_block_t *head;
} arvm_arena_t;

void *arvm_arena_alloc(arvm_arena_t *arena, size_t count);

void arvm_arena_free(arvm_arena_t *arena);

#endif /* ARENA_H */