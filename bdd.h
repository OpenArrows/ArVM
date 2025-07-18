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
  // (Empty entries have NULL function reference)
  struct {
    struct arvm_bdd_node *nodes;
    size_t length;
    size_t capacity_bits;
  } node_set;

  // Leaf node represents boolean true value. False value is obtained through
  // negation of the leaf node
  arvm_bdd_node_t leaf_node, unaligned_leaf_node;
} arvm_bdd_manager_t;

bool arvm_bdd_is_leaf(arvm_bdd_manager_t *mgr, arvm_bdd_node_t node);

arvm_bdd_node_t arvm_bdd_one(arvm_bdd_manager_t *mgr);

arvm_bdd_node_t arvm_bdd_zero(arvm_bdd_manager_t *mgr);

arvm_bdd_node_t arvm_bdd_var(arvm_bdd_manager_t *mgr, arvm_bdd_var_id_t var);

arvm_bdd_node_t arvm_bdd_ite(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b, arvm_bdd_node_t c);

arvm_bdd_node_t arvm_bdd_not(arvm_bdd_node_t a);

arvm_bdd_node_t arvm_bdd_and(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b);

arvm_bdd_node_t arvm_bdd_or(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                            arvm_bdd_node_t b);

arvm_bdd_node_t arvm_bdd_xor(arvm_bdd_manager_t *mgr, arvm_bdd_node_t a,
                             arvm_bdd_node_t b);

arvm_bdd_node_t arvm_bdd_free(arvm_bdd_manager_t *mgr);

#endif /* ARVM_BDD_H */