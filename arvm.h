#ifndef ARVM_H
#define ARVM_H

#include "bdd.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uintmax_t arvm_int_t;

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_INFINITY UINTMAX_MAX

typedef struct arvm_function *arvm_function_t;

typedef struct arvm_subdomain arvm_subdomain_t;

typedef struct arvm_operand arvm_operand_t;

typedef struct arvm_space {
  size_t size;
  arvm_function_t tail_function;
} arvm_space_t;

void arvm_dispose_space(arvm_space_t *space);

void arvm_optimize_space(arvm_space_t *space);

typedef enum arvm_state { ARVM_TODO, ARVM_VISITED, ARVM_VISITING } arvm_state_t;

// All unary boolean ArVM IR functions are piecewise defined. The single
// integer argument t is the current simulation time (tick)
struct arvm_function {
  arvm_space_t *space;

  arvm_state_t state;

  // The `counterpart` field is set on functions after mirroring operations
  // (i.e. operations that output a synonymous space)
  arvm_function_t counterpart;

  struct {
    arvm_function_t previous;
    arvm_function_t next;
  };

  arvm_subdomain_t *domain;
};

// Each subdomain (interval) of the function is defined by a truth table, with
// operands being function values at (t - C), where C is a constant positive
// integer
struct arvm_subdomain {
  arvm_int_t end;
  arvm_bdd_node_t bdd;
};

struct arvm_operand {
  arvm_function_t func;
  arvm_int_t offset;
};

arvm_function_t arvm_new_function(arvm_space_t *space);

void arvm_set_function_domain(arvm_function_t func, ...);

void arvm_delete_function(arvm_function_t func);

bool arvm_call_function(arvm_function_t func, arvm_int_t arg);

#endif /* ARVM_H */