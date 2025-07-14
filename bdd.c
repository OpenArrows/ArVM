#include "bdd.h"
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MIN_NODE_SET_CAPACITY_BITS 8

// We must manually align all pointers on systems that are not 2-aligned
#define IS_2_ALIGNED (alignof(max_align_t) >= 2)

// Hash set used to store unique BDD nodes
// (Empty entries have NULL function reference)
static struct {
  arvm_bdd_node_t *nodes;
  size_t length;
  size_t capacity_bits;
} node_set = {NULL, 0, 0};

// Leaf node represents boolean true value. False value is obtained through
// negation of the leaf node
static arvm_bdd_node_t *leaf_node = NULL, *unaligned_leaf_node = NULL;

static inline size_t node_hash(arvm_bdd_node_t node) {
  const size_t P1 = 12582917;
  const size_t P2 = 4256249;
  return (((size_t)(uintptr_t)node.lo + (size_t)node.var) * P1 +
          (size_t)(uintptr_t)node.hi * P2) >>
         (sizeof(size_t) * CHAR_BIT - node_set.capacity_bits);
}

// Inserts a new node into the set
static inline arvm_bdd_node_t *put_node(arvm_bdd_node_t node) {
  size_t capacity = 1 << node_set.capacity_bits;

  size_t i = node_hash(node);
  while (node_set.nodes[i].var != 0) {
    if (++i >= capacity)
      i = 0;
  }

  node_set.length++;
  arvm_bdd_node_t *node_ptr = node_set.nodes + i;
  if (!IS_2_ALIGNED && ((uintptr_t)node_ptr & 1))
    node_ptr++;
  *node_ptr = node;
  return node_ptr;
}

// Allocates memory for a new BDD node or returns (a negation of) an existing
// copy
static inline arvm_bdd_node_t *node_new(arvm_bdd_node_t node) {
  if (node_set.capacity_bits > 0) {
    size_t capacity = 1 << node_set.capacity_bits;

    size_t i = node_hash(node);
    arvm_bdd_node_t *existing = &node_set.nodes[i];
    while ((existing = &node_set.nodes[i++])->var != 0) {
      if (existing->var == node.var) {
        if (existing->lo == node.lo && existing->hi == node.hi)
          return existing;
        else if (existing->hi == node.lo && existing->lo == node.hi)
          return arvm_not(existing);
      }

      if (i == capacity)
        i = 0;
    }

    if (node_set.length >= capacity) {
      // Grow the set

      if (++node_set.capacity_bits > sizeof(size_t) * CHAR_BIT)
        return NULL; // overflow

      arvm_bdd_node_t *old_nodes = node_set.nodes;

      size_t new_capacity = 1 << node_set.capacity_bits;
      node_set.nodes =
          calloc(new_capacity + 1 * !IS_2_ALIGNED, sizeof(arvm_bdd_node_t));
      if (node_set.nodes == NULL)
        return NULL;

      for (size_t i = 0; i < new_capacity; i++) {
        arvm_bdd_node_t node = old_nodes[i];
        if (node.var != 0)
          put_node(node);
      }

      free(old_nodes);
    }
  } else {
    node_set.capacity_bits = MIN_NODE_SET_CAPACITY_BITS;
    node_set.nodes = calloc((1 << node_set.capacity_bits) + 1 * !IS_2_ALIGNED,
                            sizeof(arvm_bdd_node_t));
    if (node_set.nodes == NULL)
      return NULL;
  }

  return put_node(node);
}

static inline arvm_bdd_node_t *bdd_var(size_t var) {
  return node_new((arvm_bdd_node_t){var, arvm_zero(), arvm_one()});
}

// Convinience function for constructing BDD nodes
static inline arvm_bdd_node_t *bdd(size_t var, arvm_bdd_node_t *lo,
                                   arvm_bdd_node_t *hi) {
  if (lo == NULL || hi == NULL)
    return NULL;

  if (lo == arvm_one() && hi == arvm_zero())
    return arvm_not(bdd_var(var));

  if (lo == hi)
    return lo;
  return node_new((arvm_bdd_node_t){var, lo, hi});
}

// Returns the BDD leaf node
static arvm_bdd_node_t *get_leaf_node() {
  if (leaf_node == NULL) {
    leaf_node = unaligned_leaf_node = calloc(1, sizeof(arvm_bdd_node_t));
    if (!IS_2_ALIGNED && ((uintptr_t)leaf_node & 1))
      leaf_node++;
  }
  return leaf_node;
}

static inline bool is_leaf(arvm_bdd_node_t *node) {
  return (arvm_bdd_node_t *)((uintptr_t)node & ~1) == leaf_node;
}

// Public API

arvm_bdd_node_t *arvm_one() { return get_leaf_node(); }

arvm_bdd_node_t *arvm_zero() {
  return (arvm_bdd_node_t *)((uintptr_t)get_leaf_node() | 1);
}

arvm_bdd_node_t *arvm_not(arvm_bdd_node_t *a) {
  if (a == NULL)
    return NULL;

  return (arvm_bdd_node_t *)((uintptr_t)a ^ 1);
}

arvm_bdd_node_t *arvm_and(arvm_bdd_node_t *a, arvm_bdd_node_t *b) {
  if (a == NULL || b == NULL)
    return NULL;

  if (a == arvm_zero() || b == arvm_zero())
    return arvm_zero();

  if (a == arvm_one())
    return b;
  else if (b == arvm_one())
    return a;

  if (a->var < b->var)
    return bdd(b->var, arvm_and(a, b->lo), arvm_and(a, b->hi));
  else if (a->var < b->var)
    return bdd(a->var, arvm_and(a->lo, b), arvm_and(a->hi, b));
  else
    return bdd(a->var, arvm_and(a->lo, b->lo), arvm_and(a->hi, b->hi));
}

arvm_bdd_node_t *arvm_or(arvm_bdd_node_t *a, arvm_bdd_node_t *b) {
  return arvm_not(arvm_and(arvm_not(a), arvm_not(b)));
}

arvm_bdd_node_t *arvm_xor(arvm_bdd_node_t *a, arvm_bdd_node_t *b) {
  if (a == NULL || b == NULL)
    return NULL;

  if (is_leaf(a) && is_leaf(b))
    return a == b ? arvm_zero() : arvm_one();

  if (a->var < b->var)
    return bdd(b->var, arvm_xor(a, b->lo), arvm_xor(a, b->hi));
  else if (a->var < b->var)
    return bdd(a->var, arvm_xor(a->lo, b), arvm_xor(a->hi, b));
  else
    return bdd(a->var, arvm_xor(a->lo, b->lo), arvm_xor(a->hi, b->hi));
}