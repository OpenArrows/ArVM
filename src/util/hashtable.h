#ifndef ARVM_HASHTABLE_H
#define ARVM_HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>

typedef size_t (*arvm_hashf_t)(const void *);

typedef int (*arvm_cmpf_t)(const void *, const void *);

typedef struct arvm_hashtable {
  size_t min_capacity;
  arvm_hashf_t hashf;
  arvm_cmpf_t cmpf;

  size_t capacity /* must be power of 2 */, length;
  struct arvm_ht_entry *entries;
} arvm_hashtable_t;

#define ARVM_HASHTABLE(min_capacity, hashf, cmpf)                              \
  ((arvm_hashtable_t){(min_capacity), (hashf), (cmpf)})

void *arvm_ht_get(arvm_hashtable_t *ht, const void *key, bool *found);

bool arvm_ht_set(arvm_hashtable_t *ht, void *key, void *value);

bool arvm_ht_del(arvm_hashtable_t *ht, const void *key);

void arvm_ht_free(arvm_hashtable_t *ht);

#endif /* ARVM_HASHTABLE_H */