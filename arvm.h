#ifndef ARVM_H
#define ARVM_H

#include "bdd.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uintmax_t arvm_int_t;

#define ARVM_INFINITY UINTMAX_MAX

typedef struct arvm_function *arvm_function_t;

typedef struct arvm_subdomain arvm_subdomain_t;

typedef arvm_bdd_node_t arvm_expr_t;

typedef struct arvm_space {
  size_t size;
  arvm_function_t tail_function;

  arvm_bdd_manager_t bdd_mgr;
  struct {
    struct arvm_bdd_var_entry *entries;
    size_t length;
    size_t capacity_bits;
  } var_map;
  struct arvm_bdd_var *vars;
  size_t var_index;
} arvm_space_t;

void arvm_dispose_space(arvm_space_t *space);

void arvm_update(arvm_space_t *space, arvm_int_t dt);

// Each subdomain (interval) of a function is defined by a truth table, with
// operands being function values at (t - C), where C is a constant positive
// integer
struct arvm_subdomain {
  arvm_int_t end;
  arvm_expr_t value;
};

arvm_function_t arvm_new_function(arvm_space_t *space);

arvm_expr_t arvm_true();

arvm_expr_t arvm_false();

arvm_expr_t arvm_make_call(arvm_space_t *space, arvm_function_t callee);

arvm_expr_t arvm_not(arvm_expr_t a);

arvm_expr_t arvm_make_ite(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b,
                          arvm_expr_t c);

arvm_expr_t arvm_make_and(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b);

arvm_expr_t arvm_make_or(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b);

arvm_expr_t arvm_make_xor(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b);

void arvm_set_function_domain(arvm_function_t func, ...);

void arvm_delete_function(arvm_function_t func);

bool arvm_get_function_value(arvm_function_t func);

#endif /* ARVM_H */