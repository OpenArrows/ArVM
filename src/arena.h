#include <stdint.h>

#ifndef ARENA
#define ARENA

typedef struct arena_block arena_block_t;

struct arena_block {
  arena_block_t *next;
  size_t allocated;
  size_t capacity;
};

typedef struct arena {
  size_t block_size;

  arena_block_t *head;
} arena_t;

void *arena_alloc(arena_t *arena, size_t count);

void arena_free(arena_t *arena);
#endif