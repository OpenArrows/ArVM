#include "arena.h"
#include <stdlib.h>

void *arena_alloc(arena_t *arena, size_t count) {
  arena_block_t **block = &arena->head;
  while (*block != NULL) {
    if ((*block)->capacity - (*block)->allocated >= count) {
      void *ptr = (void *)(*block + 1) + (*block)->allocated;
      (*block)->allocated += count;
      return ptr;
    } else {
      block = &(*block)->next;
    }
  }
  size_t size =
      arena->block_size + (count - 1) / arena->block_size * arena->block_size;
  arena_block_t *new_block = malloc(sizeof(arena_block_t) + size);
  if (new_block == NULL)
    return NULL;
  new_block->next = NULL;
  new_block->allocated = count;
  new_block->capacity = size;
  *block = new_block;
  return new_block + 1;
}

void arena_free(arena_t *arena) {
  arena_block_t *block = arena->head;
  arena->head = NULL;
  while (block != NULL) {
    arena_block_t *next = block->next;
    free(block);
    block = next;
  }
}