#ifndef ARVM_BDD_H
#define ARVM_BDD_H

#include <stdbool.h>

typedef size_t arvm_bdd_var_id_t;

typedef struct arvm_bdd_node *arvm_bdd_node_t;

struct arvm_bdd_node {
  arvm_bdd_var_id_t var;
  arvm_bdd_node_t lo;
  arvm_bdd_node_t hi;
};

typedef struct arvm_bdd_manager {
  // Hash set used to store unique BDD nodes
  // (Empty entries have NULL children)
  struct {
    struct arvm_bdd_node *nodes;
    size_t length;
    size_t capacity_bits;
  } node_set;
} arvm_bdd_manager_t;

#define _ARVM_BDD_NORM(node) ((arvm_bdd_node_t)((uintptr_t)(node) & ~1))

#define _ARVM_BDD_IS_NEG(node) ((bool)((uintptr_t)(node) & 1))

#define ARVM_BDD_NOT(node) ((arvm_bdd_node_t)((uintptr_t)(node) ^ 1))

#define ARVM_BDD_TRUE ((arvm_bdd_node_t)NULL)

#define ARVM_BDD_FALSE ARVM_BDD_NOT(ARVM_BDD_TRUE)

#define ARVM_BDD_IS_LEAF(mgr, node)                                            \
  ((node) == ARVM_BDD_TRUE || (node) == ARVM_BDD_FALSE)

#define ARVM_BDD_VAR(node) (_ARVM_BDD_NORM(node)->var)

#define ARVM_BDD_LO(node)                                                      \
  (_ARVM_BDD_IS_NEG(node) ? _ARVM_BDD_NORM(node)->hi : _ARVM_BDD_NORM(node)->lo)

#define ARVM_BDD_HI(node)                                                      \
  (_ARVM_BDD_IS_NEG(node) ? _ARVM_BDD_NORM(node)->lo : _ARVM_BDD_NORM(node)->hi)

arvm_bdd_node_t arvm_bdd_var(arvm_bdd_manager_t *mgr, arvm_bdd_var_id_t var);

arvm_bdd_node_t arvm_bdd_restrict(arvm_bdd_manager_t *mgr, arvm_bdd_node_t f,
                                  arvm_bdd_var_id_t var, bool val);

arvm_bdd_node_t arvm_bdd_ite(arvm_bdd_manager_t *mgr, arvm_bdd_node_t f,
                             arvm_bdd_node_t g, arvm_bdd_node_t h);

void arvm_bdd_free(arvm_bdd_manager_t *mgr);

#endif /* ARVM_BDD_H */