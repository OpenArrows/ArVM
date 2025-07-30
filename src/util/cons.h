#ifndef ARVM_CONS_H
#define ARVM_CONS_H

#include "hashtable.h"
#include "pool.h"

struct arvm_cons_table_entry {
  size_t refs;
  char data[];
};

typedef struct arvm_cons_table {
  arvm_pool_t pool;
  arvm_hashtable_t hash_table;
} arvm_cons_table_t;

#define ARVM_CONS_TABLE(object_size, block_size, hashf, cmpf)                  \
  ((arvm_cons_table_t){                                                        \
      .pool =                                                                  \
          ARVM_POOL(sizeof(struct arvm_cons_table_entry) +                     \
                        sizeof(struct arvm_cons_table_entry) + (object_size),  \
                    (block_size), NULL),                                       \
      .hash_table = ARVM_HASHTABLE((block_size), (hashf), (cmpf))})

// Constructs a new object in memory or returns an already existing isomorphic
// copy
void *arvm_cons(arvm_cons_table_t *table, const void *data);

// Tries to find an existing copy of the object without constructing a new one
void *arvm_cons_find(arvm_cons_table_t *table, const void *data);

// Creates an object reference
void arvm_cons_ref(arvm_cons_table_t *table, void *object);

// Removes the reference to an object and returns a boolean value indicating
// whether the object has no more references. If there are no references
// remaining, the object is destroyed and the memory is freed
bool arvm_cons_free(arvm_cons_table_t *table, void *object);

// Clears the given `cons` table
void arvm_cons_clear(arvm_cons_table_t *table);

#endif /* ARVM_CONS_H */