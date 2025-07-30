#include "bdd.h"
#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t arvm_bdd_node_hash(const void *node_) {
  const struct arvm_bdd_node *node = node_;
  const size_t P1 = 12582917;
  const size_t P2 = 4256249;
  return ((size_t)node->lo + (size_t)node->var) * P1 + (size_t)node->hi * P2;
}

int arvm_bdd_node_cmp(const void *a_, const void *b_) {
  const struct arvm_bdd_node *a = a_;
  const struct arvm_bdd_node *b = b_;
  if (a->lo != b->lo)
    return (a->lo < b->lo) - (a->lo > b->lo);
  else if (a->hi != b->hi)
    return (a->hi < b->hi) - (a->hi > b->hi);
  else if (a->var != b->var)
    return (a->var < b->var) - (a->var > b->var);
  else
    return 0;
}

// Creates a new BDD node or returns (a negation of) an existing copy
static arvm_bdd_node_t bdd(arvm_bdd_manager_t *mgr, size_t var,
                           arvm_bdd_node_t hi, arvm_bdd_node_t lo) {
  if (lo == hi)
    return lo;

  arvm_bdd_ref_node(mgr, lo);
  arvm_bdd_ref_node(mgr, hi);

  arvm_bdd_node_t inverse =
      arvm_cons_find(&mgr->node_table, &(struct arvm_bdd_node){var, hi, lo});
  if (inverse != NULL)
    return ARVM_BDD_NOT(inverse);

  return arvm_cons(&mgr->node_table, &(struct arvm_bdd_node){var, lo, hi});
}

// Public API

arvm_bdd_node_t arvm_bdd_var(arvm_bdd_manager_t *mgr, arvm_bdd_var_id_t var) {
  return bdd(mgr, var, ARVM_BDD_TRUE, ARVM_BDD_FALSE);
}

arvm_bdd_node_t arvm_bdd_restrict(arvm_bdd_manager_t *mgr, arvm_bdd_node_t f,
                                  arvm_bdd_var_id_t var, bool val) {
  if (ARVM_BDD_IS_LEAF(f))
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

// TODO: fix memory leaks
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
  if (!ARVM_BDD_IS_LEAF(g) && ARVM_BDD_VAR(g) < split)
    split = ARVM_BDD_VAR(g);
  if (!ARVM_BDD_IS_LEAF(h) && ARVM_BDD_VAR(h) < split)
    split = ARVM_BDD_VAR(h);

  return bdd(mgr, split,
             arvm_bdd_ite(mgr, arvm_bdd_restrict(mgr, f, split, true),
                          arvm_bdd_restrict(mgr, g, split, true),
                          arvm_bdd_restrict(mgr, h, split, true)),
             arvm_bdd_ite(mgr, arvm_bdd_restrict(mgr, f, split, false),
                          arvm_bdd_restrict(mgr, g, split, false),
                          arvm_bdd_restrict(mgr, h, split, false)));
}

void arvm_bdd_ref_node(arvm_bdd_manager_t *mgr, arvm_bdd_node_t node) {
  if (ARVM_BDD_IS_LEAF(node))
    return;

  arvm_cons_ref(&mgr->node_table, _ARVM_BDD_NORM(node));
}

void arvm_bdd_free_node(arvm_bdd_manager_t *mgr, arvm_bdd_node_t node) {
  if (ARVM_BDD_IS_LEAF(node))
    return;

  arvm_bdd_node_t lo = node->lo, hi = node->hi;
  if (arvm_cons_free(&mgr->node_table, _ARVM_BDD_NORM(node))) {
    arvm_bdd_free_node(mgr, lo);
    arvm_bdd_free_node(mgr, hi);
  }
}

void arvm_bdd_free(arvm_bdd_manager_t *mgr) {
  arvm_cons_clear(&mgr->node_table);
}