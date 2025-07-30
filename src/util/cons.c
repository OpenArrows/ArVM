#include "cons.h"
#include "meta.h"
#include <stdlib.h>
#include <string.h>

void *arvm_cons(arvm_cons_table_t *table, const void *data) {
  void *existing = arvm_cons_find(table, data);
  if (existing != NULL)
    return existing;

  // No existing object found, put a new entry into the table
  struct arvm_cons_table_entry *entry = arvm_pool_alloc(&table->pool);
  if (entry == NULL)
    return NULL;
  memcpy(entry->data, data,
         table->pool.item_size - sizeof(struct arvm_cons_table_entry));
  entry->refs = 0;
  if (!arvm_ht_set(&table->hash_table, entry->data, entry)) {
    arvm_pool_free(&table->pool, entry);
    return NULL;
  }
  return entry->data;
}

void *arvm_cons_find(arvm_cons_table_t *table, const void *data) {
  // Perform hash table lookup to find an isomorphic object
  struct arvm_cons_table_entry *entry =
      arvm_ht_get(&table->hash_table, data, NULL);
  return entry != NULL ? &entry->data : NULL;
}

void arvm_cons_ref(arvm_cons_table_t *table, void *object) {
  container_of(object, struct arvm_cons_table_entry, data)->refs++;
}

bool arvm_cons_free(arvm_cons_table_t *table, void *object) {
  struct arvm_cons_table_entry *entry =
      container_of(object, struct arvm_cons_table_entry, data);
  if (--entry->refs == 0) {
    // No references, delete the object
    arvm_ht_del(&table->hash_table, object);
    arvm_pool_free(&table->pool, entry);
    return true;
  }
  return false;
}

void arvm_cons_clear(arvm_cons_table_t *table) {
  arvm_ht_free(&table->hash_table);
  arvm_pool_clear(&table->pool);
}