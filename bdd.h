#ifndef ARVM_BDD_H
#define ARVM_BDD_H

#include <stdbool.h>

typedef struct arvm_bdd_node *arvm_bdd_node_t;

typedef size_t arvm_var_index_t;

struct arvm_bdd_node {
  arvm_var_index_t var;
  arvm_bdd_node_t lo;
  arvm_bdd_node_t hi;
};

bool arvm_is_const(arvm_bdd_node_t node);

arvm_bdd_node_t arvm_one();

arvm_bdd_node_t arvm_zero();

arvm_bdd_node_t arvm_var(arvm_var_index_t var);

arvm_bdd_node_t arvm_ite(arvm_bdd_node_t a, arvm_bdd_node_t b,
                         arvm_bdd_node_t c);

arvm_bdd_node_t arvm_not(arvm_bdd_node_t a);

arvm_bdd_node_t arvm_and(arvm_bdd_node_t a, arvm_bdd_node_t b);

arvm_bdd_node_t arvm_or(arvm_bdd_node_t a, arvm_bdd_node_t b);

arvm_bdd_node_t arvm_xor(arvm_bdd_node_t a, arvm_bdd_node_t b);

#endif /* ARVM_BDD_H */