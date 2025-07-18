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

static inline size_t node_hash(arvm_bdd_manager_t *mgr,
                               struct arvm_bdd_node node) {
  const size_t P1 = 12582917;
  const size_t P2 = 4256249;
  return (((size_t)(uintptr_t)node.lo + (size_t)node.var) * P1 +
          (size_t)(uintptr_t)node.hi * P2) >>
         (sizeof(size_t) * CHAR_BIT - mgr->node_set.capacity_bits);
}

// Inserts a new node into the set
static inline arvm_bdd_node_t put_node(arvm_bdd_manager_t *mgr,
                                       struct arvm_bdd_node node) {
  size_t capacity = 1 << mgr->node_set.capacity_bits;

  size_t i = node_hash(mgr, node);
  while (mgr->node_set.nodes[i].var != 0) {
    if (++i >= capacity)
      i = 0;
  }

  mgr->node_set.length++;
  arvm_bdd_node_t node_ptr = mgr->node_set.nodes + i;
  if (!IS_2_ALIGNED && ((uintptr_t)node_ptr & 1))
    node_ptr++;
  *node_ptr = node;
  return node_ptr;
}

// Allocates memory for a new BDD node or returns (a negation of) an existing
// copy
static inline arvm_bdd_node_t node_new(arvm_bdd_manager_t *mgr,
                                       struct arvm_bdd_node node) {
  if (mgr->node_set.capacity_bits > 0) {
    size_t capacity = 1 << mgr->node_set.capacity_bits;

    size_t i = node_hash(mgr, node);
    arvm_bdd_node_t existing;
    while ((existing = &mgr->node_set.nodes[i++])->var != 0) {
      if (existing->var == node.var && existing->lo == node.lo &&
          existing->hi == node.hi)
        return existing;

      if (i >= capacity)
        i = 0;
    }

    i = node_hash(mgr, (struct arvm_bdd_node){node.var, node.hi, node.lo});
    while ((existing = &mgr->node_set.nodes[i++])->var != 0) {
      if (existing->var == node.var && existing->lo == node.hi &&
          existing->hi == node.lo)
        return arvm_bdd_not(existing);

      if (i >= capacity)
        i = 0;
    }

    if (mgr->node_set.length >= capacity) {
      // Grow the set

      if (mgr->node_set.capacity_bits + 1 > sizeof(size_t) * CHAR_BIT)
        return NULL; // overflow

      size_t new_capacity = 1 << (mgr->node_set.capacity_bits + 1);
      struct arvm_bdd_node *new_nodes = calloc(new_capacity + 1 * !IS_2_ALIGNED,
                                               sizeof(struct arvm_bdd_node));
      if (new_nodes == NULL)
        return NULL;

      struct arvm_bdd_node *old_nodes = mgr->node_set.nodes;
      mgr->node_set.nodes = new_nodes;
      mgr->node_set.length = 0;
      mgr->node_set.capacity_bits++;

      for (size_t i = 0; i < new_capacity; i++) {
        struct arvm_bdd_node node = old_nodes[i];
        if (node.var != 0)
          put_node(mgr, node);
      }

      free(old_nodes);
    }
  } else {
    mgr->node_set.nodes =
        calloc((1 << MIN_NODE_SET_CAPACITY_BITS) + 1 * !IS_2_ALIGNED,
               sizeof(struct arvm_bdd_node));
    if (mgr->node_set.nodes == NULL)
      return NULL;
    mgr->node_set.capacity_bits = MIN_NODE_SET_CAPACITY_BITS;
  }

  return put_node(mgr, node);
}

// Convinience function for constructing BDD nodes
static inline arvm_bdd_node_t bdd(arvm_bdd_manager_t *mgr, size_t var,
                                  arvm_bdd_node_t lo, arvm_bdd_node_t hi) {
  if (lo == NULL || hi == NULL)
    return NULL;

  if (lo == arvm_bdd_one(mgr) && hi == arvm_bdd_zero(mgr))
    return arvm_bdd_not(node_new(mgr, (struct arvm_bdd_node){var, hi, lo}));

  if (lo == hi)
    return lo;
  return node_new(mgr, (struct arvm_bdd_node){var, lo, hi});
}

// Returns the BDD leaf node
static arvm_bdd_node_t get_leaf_node(arvm_bdd_manager_t *mgr) {
  if (mgr->leaf_node == NULL) {
    mgr->leaf_node = mgr->unaligned_leaf_node =
        calloc(1, sizeof(struct arvm_bdd_node));
    if (!IS_2_ALIGNED && ((uintptr_t)mgr->leaf_node & 1))
      mgr->leaf_node++;
  }
  return mgr->leaf_node;
}

// Public API

bool arvm_bdd_is_leaf(arvm_bdd_manager_t *mgr, arvm_bdd_node_t node) {
  return (arvm_bdd_node_t)((uintptr_t)node & ~1) == mgr->leaf_node;
}

arvm_bdd_node_t arvm_bdd_one(arvm_bdd_manager_t *mgr) {
  return get_leaf_node(mgr);
}

arvm_bdd_node_t arvm_bdd_zero(arvm_bdd_manager_t *mgr) {
  return (arvm_bdd_node_t)((uintptr_t)get_leaf_node(mgr) | 1);
}

arvm_bdd_node_t arvm_bdd_var(arvm_bdd_manager_t *mgr, arvm_bdd_var_id_t var) {
  return bdd(mgr, var, arvm_bdd_zero(mgr), arvm_bdd_one(mgr));
}

arvm_bdd_node_t arvm_bdd_not(arvm_bdd_node_t a) {
  if (a == NULL)
    return NULL;

  return (arvm_bdd_node_t)((uintptr_t)a ^ 1);
}

arvm_bdd_node_t arvm_bdd_ite(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b, arvm_bdd_node_t c) {
  if (a == NULL || b == NULL || c == NULL)
    return NULL;

  if (a == arvm_bdd_one(mgr))
    return b;
  else if (a == arvm_bdd_zero(mgr))
    return c;

  if (b == c)
    return b;

  arvm_bdd_var_id_t v = a->var;
  if (b->var < v)
    v = b->var;
  if (c->var < v)
    v = c->var;

  return bdd(mgr, v,
             arvm_bdd_ite(mgr, v < a->var ? a : a->hi, v < b->var ? b : b->hi,
                          v < c->var ? c : c->hi),
             arvm_bdd_ite(mgr, v < a->var ? a : a->lo, v < b->var ? b : b->lo,
                          v < c->var ? c : c->lo));
}

arvm_bdd_node_t arvm_bdd_and(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b) {
  if (a == NULL || b == NULL)
    return NULL;

  if (a == arvm_bdd_zero(mgr) || b == arvm_bdd_zero(mgr))
    return arvm_bdd_zero(mgr);

  if (a == arvm_bdd_one(mgr))
    return b;
  else if (b == arvm_bdd_one(mgr))
    return a;

  if (a->var < b->var)
    return bdd(mgr, b->var, arvm_bdd_and(mgr, a, b->lo),
               arvm_bdd_and(mgr, a, b->hi));
  else if (a->var < b->var)
    return bdd(mgr, a->var, arvm_bdd_and(mgr, a->lo, b),
               arvm_bdd_and(mgr, a->hi, b));
  else
    return bdd(mgr, a->var, arvm_bdd_and(mgr, a->lo, b->lo),
               arvm_bdd_and(mgr, a->hi, b->hi));
}

arvm_bdd_node_t arvm_bdd_or(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                            arvm_bdd_node_t b) {
  return arvm_bdd_not(arvm_bdd_and(mgr, arvm_bdd_not(a), arvm_bdd_not(b)));
}

arvm_bdd_node_t arvm_bdd_xor(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b) {
  if (a == NULL || b == NULL)
    return NULL;

  if (a == arvm_bdd_zero(mgr))
    return b;
  else if (b == arvm_bdd_zero(mgr))
    return a;

  if (a == arvm_bdd_one(mgr))
    return arvm_bdd_not(b);
  else if (b == arvm_bdd_one(mgr))
    return arvm_bdd_not(a);

  if (a->var < b->var)
    return bdd(mgr, b->var, arvm_bdd_xor(mgr, a, b->lo),
               arvm_bdd_xor(mgr, a, b->hi));
  else if (a->var < b->var)
    return bdd(mgr, a->var, arvm_bdd_xor(mgr, a->lo, b),
               arvm_bdd_xor(mgr, a->hi, b));
  else
    return bdd(mgr, a->var, arvm_bdd_xor(mgr, a->lo, b->lo),
               arvm_bdd_xor(mgr, a->hi, b->hi));
}

arvm_bdd_node_t arvm_bdd_free(arvm_bdd_manager_t *mgr) {
  free(mgr->unaligned_leaf_node);
  mgr->leaf_node = mgr->unaligned_leaf_node = NULL;

  free(mgr->node_set.nodes);
  mgr->node_set.capacity_bits = 0;
  mgr->node_set.length = 0;
  mgr->node_set.nodes = NULL;
}