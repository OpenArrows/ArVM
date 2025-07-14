#ifndef ARVM_BDD_H
#define ARVM_BDD_H

#include "arvm.h"

typedef struct arvm_bdd_node arvm_bdd_node_t;

struct arvm_bdd_node {
  size_t var;
  arvm_bdd_node_t *lo;
  arvm_bdd_node_t *hi;
};

arvm_bdd_node_t *arvm_one();

arvm_bdd_node_t *arvm_zero();

arvm_bdd_node_t *arvm_not(arvm_bdd_node_t *a);

arvm_bdd_node_t *arvm_and(arvm_bdd_node_t *a, arvm_bdd_node_t *b);

arvm_bdd_node_t *arvm_or(arvm_bdd_node_t *a, arvm_bdd_node_t *b);

arvm_bdd_node_t *arvm_xor(arvm_bdd_node_t *a, arvm_bdd_node_t *b);

#endif /* ARVM_BDD_H */