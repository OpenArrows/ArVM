#include "pool.h"
#include "bdd.h"
#include "meta.h"
#include <stdlib.h>

bool arvm_pool_reserve(arvm_pool_t *pool, size_t count) {
  if (count <= pool->free_count)
    return true;

  count -= pool->free_count;

  arvm_pool_item_t *items = NULL;
  arvm_pool_block_t *new_blocks = NULL;

  size_t item_count = (1 + (count - 1) / pool->block_size) * pool->block_size;
  items = malloc(pool->item_size * item_count);
  if (items == NULL)
    goto fail;

  new_blocks = realloc(pool->blocks,
                       sizeof(arvm_pool_block_t) * (pool->block_count + 1));
  if (new_blocks == NULL)
    goto fail;
  pool->blocks = new_blocks;

  pool->blocks[pool->block_count++] = (arvm_pool_block_t){items};

  for (size_t i = 0; i < item_count; i++) {
    arvm_pool_item_t *item =
        item_at(items, arvm_pool_item_t, i, pool->item_size);
    if (pool->initializer != NULL) {
      if (!pool->initializer(pool, item))
        goto fail;
    }
    *(arvm_pool_item_t **)item =
        i + 1 < item_count
            ? item_at(items, arvm_pool_item_t, i + 1, pool->item_size)
            : NULL;
  }

  pool->free_item = item_at(items, arvm_pool_item_t, 0, pool->item_size);
  pool->free_count += item_count;

  return true;

fail:
  free(items);
  free(new_blocks);
  return false;
}

void *arvm_pool_alloc(arvm_pool_t *pool) {
  if (pool->free_item == NULL && !arvm_pool_reserve(pool, 1))
    return NULL;

  arvm_pool_item_t *item = pool->free_item;
  pool->free_item = *(arvm_pool_item_t **)item;
  pool->free_count--;
  return item;
}

void arvm_pool_free(arvm_pool_t *pool, void *ptr) {
  arvm_pool_item_t *item = ptr;
  *(arvm_pool_item_t **)item = pool->free_item;
  pool->free_item = item;
  pool->free_count++;
}

void arvm_pool_clear(arvm_pool_t *pool) {
  for (size_t i = 0; i < pool->block_count; i++)
    free(pool->blocks[i].items);
  free(pool->blocks);

  pool->free_item = NULL;
  pool->free_count = 0;
  pool->block_count = 0;
}