#include "hashtable.h"
#include <stdint.h>
#include <stdlib.h>

struct arvm_ht_entry {
  void *key;
  void *value;
};

// Finds an available hash table entry for a given key
static inline struct arvm_ht_entry *lookup(arvm_hashtable_t *ht,
                                           const void *key) {
  if (ht->capacity == 0)
    return NULL;

  size_t i = ht->hashf(key) & (ht->capacity - 1);

  struct arvm_ht_entry *entry;
  while ((entry = &ht->entries[i])->key != NULL) {
    if (ht->cmpf(entry->key, key) == 0)
      break;

    if (++i >= ht->capacity)
      i = 0;
  }
  return entry;
}

void *arvm_ht_get(arvm_hashtable_t *ht, const void *key, bool *found) {
  struct arvm_ht_entry *entry = lookup(ht, key);
  bool is_valid = entry != NULL && entry->key != NULL;
  if (found != NULL)
    *found = is_valid;
  return is_valid ? entry->value : NULL;
}

bool arvm_ht_set(arvm_hashtable_t *ht, void *key, void *value) {
  if (ht->capacity == 0) { // initialize the table if it's empty
    // Round min_capacity up to a power of 2
    size_t capacity = ht->min_capacity;
    if (capacity == 0)
      capacity = 1;
    else {
      capacity--;
      if (SIZE_MAX >= UINT8_MAX) {
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
      }
      if (SIZE_MAX >= UINT16_MAX)
        capacity |= capacity >> 8;
      if (SIZE_MAX >= UINT32_MAX)
        capacity |= capacity >> 16;
      if (SIZE_MAX >= UINT64_MAX)
        capacity |= capacity >> 32;
      capacity++;
    }

    ht->entries = calloc(capacity, sizeof(struct arvm_ht_entry));
    if (ht->entries == NULL)
      return false;
    ht->capacity = capacity;
  } else if (ht->length >= ht->capacity / 2) { // expand the table if needed
    size_t new_capacity = ht->capacity * 2;
    if (new_capacity < ht->capacity)
      return false; // integer overflow

    struct arvm_ht_entry *new_entries =
        calloc(new_capacity, sizeof(struct arvm_ht_entry));
    if (new_entries == NULL)
      return false;

    struct arvm_ht_entry *old_entries = ht->entries;
    size_t old_capacity = ht->capacity;

    ht->entries = new_entries;
    ht->capacity = new_capacity;

    for (size_t i = 0; i < old_capacity; i++) {
      struct arvm_ht_entry entry = old_entries[i];
      if (entry.key != NULL)
        *lookup(ht, entry.key) = entry;
    }
    free(old_entries);
  }

  struct arvm_ht_entry *entry = lookup(ht, key);
  if (entry->key == NULL) {
    ht->length++;
    entry->key = key;
  }
  entry->value = value;
  return true;
}

bool arvm_ht_del(arvm_hashtable_t *ht, const void *key) {
  struct arvm_ht_entry *entry = lookup(ht, key);
  if (entry == NULL || entry->key == NULL)
    return false;

  entry->key = NULL;
  entry->value = NULL;
  return true;
}

void arvm_ht_free(arvm_hashtable_t *ht) {
  ht->capacity = ht->length = 0;
  free(ht->entries);
}