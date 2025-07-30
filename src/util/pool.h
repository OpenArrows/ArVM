#ifndef ARVM_POOL_H
#define ARVM_POOL_H

#include <stdbool.h>
#include <stddef.h>

// If a pool item is free, it contains a pointer to the next free item.
// Otherwise, it is filled with user data
typedef void arvm_pool_item_t;

// To store persistent data (i.e. data that doesn't get erased or overwritten
// between `arvm_pool_alloc`/`arvm_pool_free` calls) the user needs to manually
// reserve memory for internal metadata:
// ```
// struct {
//   union {
//     arvm_pool_reserved_t _reserved0;
//     struct { /* ... */ } current_data;
//   };
//   struct { /* ... */ } persistent_data;
// };
// ```
typedef struct {
  arvm_pool_item_t *_0;
} arvm_pool_reserved_t;

// Each pool block points to a contiguous array of items
typedef struct arvm_pool_block {
  arvm_pool_item_t *items;
} arvm_pool_block_t;

// Pool allocator allocates blocks of memory for objects and then reuses
// these blocks until it is cleared
typedef struct arvm_pool arvm_pool_t;

// Initializes a pool item when it is first created
typedef bool (*arvm_pool_initializer_t)(arvm_pool_t *, void *);

struct arvm_pool {
  size_t item_size, block_size;
  arvm_pool_initializer_t initializer;

  arvm_pool_item_t *free_item;
  size_t free_count;

  size_t block_count;
  arvm_pool_block_t *blocks;
};

#define ARVM_POOL(item_size, block_size, initializer)                          \
  ((arvm_pool_t){(item_size), (block_size), (initializer)})

// Reserves memory for a specified amount of objects and returns a boolean value
// indicating success
bool arvm_pool_reserve(arvm_pool_t *pool, size_t count);

void *arvm_pool_alloc(arvm_pool_t *pool); // returns NULL if allocation fails

void arvm_pool_free(arvm_pool_t *pool, void *ptr);

void arvm_pool_clear(arvm_pool_t *pool);

#endif /* ARVM_POOL_H */