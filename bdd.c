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
  while (mgr->node_set.nodes[i].lo != NULL) {
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
// TODO: handle malloc failure
static inline arvm_bdd_node_t node_new(arvm_bdd_manager_t *mgr,
                                       struct arvm_bdd_node node) {
  if (mgr->node_set.capacity_bits > 0) {
    size_t capacity = 1 << mgr->node_set.capacity_bits;

    size_t i = node_hash(mgr, node);
    arvm_bdd_node_t existing;
    while ((existing = &mgr->node_set.nodes[i++])->lo != NULL) {
      if (existing->var == node.var && existing->lo == node.lo &&
          existing->hi == node.hi)
        return existing;

      if (i >= capacity)
        i = 0;
    }

    i = node_hash(mgr, (struct arvm_bdd_node){node.var, node.hi, node.lo});
    while ((existing = &mgr->node_set.nodes[i++])->lo != NULL) {
      if (existing->var == node.var && existing->lo == node.hi &&
          existing->hi == node.lo)
        return ARVM_BDD_NOT(existing);

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
        if (node.lo != NULL)
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
                                  arvm_bdd_node_t hi, arvm_bdd_node_t lo) {
  if (lo == hi)
    return lo;
  return node_new(mgr, (struct arvm_bdd_node){var, lo, hi});
}

// Public API

arvm_bdd_node_t arvm_bdd_var(arvm_bdd_manager_t *mgr, arvm_bdd_var_id_t var) {
  return bdd(mgr, var, ARVM_BDD_TRUE, ARVM_BDD_FALSE);
}

arvm_bdd_node_t arvm_bdd_restrict(arvm_bdd_manager_t *mgr, arvm_bdd_node_t f,
                                  arvm_bdd_var_id_t var, bool val) {
  if (ARVM_BDD_IS_LEAF(mgr, f))
    return f;

  arvm_bdd_var_id_t x = ARVM_BDD_VAR(f);
  if (x > var)
    return f;
  else if (x < var)
    return bdd(mgr, x, arvm_bdd_restrict(mgr, ARVM_BDD_HI(f), var, val),
               arvm_bdd_restrict(mgr, ARVM_BDD_LO(f), var, val));
  else
    // TODO: can this be just `val ? hi : lo`?
    return val ? arvm_bdd_restrict(mgr, ARVM_BDD_HI(f), var, val)
               : arvm_bdd_restrict(mgr, ARVM_BDD_LO(f), var, val);
}

arvm_bdd_node_t arvm_bdd_ite(arvm_bdd_manager_t *mgr, arvm_bdd_node_t f,
                             arvm_bdd_node_t g, arvm_bdd_node_t h) {
  if (f == ARVM_BDD_TRUE)
    return g;
  else if (f == ARVM_BDD_FALSE)
    return h;

  if (g == h)
    return g;

  if (g == ARVM_BDD_TRUE && h == ARVM_BDD_FALSE)
    return f;
  else if (g == ARVM_BDD_FALSE && h == ARVM_BDD_TRUE)
    return ARVM_BDD_NOT(f);

  arvm_bdd_var_id_t split = ARVM_BDD_VAR(f);
  if (ARVM_BDD_VAR(g) < split)
    split = ARVM_BDD_VAR(g);
  if (ARVM_BDD_VAR(h) < split)
    split = ARVM_BDD_VAR(h);

  return bdd(mgr, split,
             arvm_bdd_ite(mgr, arvm_bdd_restrict(mgr, f, split, true),
                          arvm_bdd_restrict(mgr, g, split, true),
                          arvm_bdd_restrict(mgr, h, split, true)),
             arvm_bdd_ite(mgr, arvm_bdd_restrict(mgr, f, split, false),
                          arvm_bdd_restrict(mgr, g, split, false),
                          arvm_bdd_restrict(mgr, h, split, false)));
}

void arvm_bdd_free(arvm_bdd_manager_t *mgr) {
  free(mgr->node_set.nodes);
  mgr->node_set.capacity_bits = 0;
  mgr->node_set.length = 0;
  mgr->node_set.nodes = NULL;
}