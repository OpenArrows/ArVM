#ifndef ARVM_H
#define ARVM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARVM_FUNC_BLOCK_SIZE 256

typedef uintmax_t arvm_int_t;

#define ARVM_INFINITY UINTMAX_MAX

// Space holds and updates ArVM functions
typedef struct arvm_space *arvm_space_t;

// All unary boolean ArVM IR functions are piecewise defined. The single
// integer argument t is the current simulation time (tick)
typedef struct arvm_function *arvm_function_t;

typedef struct arvm_subdomain arvm_subdomain_t;

typedef struct arvm_bdd_node *arvm_expr_t;

arvm_space_t arvm_create_space();

void arvm_dispose_space(arvm_space_t space);

void arvm_update(arvm_space_t space, arvm_int_t dt);

bool arvm_reserve(arvm_space_t space, size_t size);

// Each subdomain (interval) of a function is defined by a truth table, with
// operands being function values at (t - C), where C is a constant positive
// integer
struct arvm_subdomain {
  arvm_int_t end;
  arvm_expr_t value;
};

arvm_function_t arvm_new_function(arvm_space_t space);

arvm_expr_t arvm_true();

arvm_expr_t arvm_false();

arvm_expr_t arvm_make_call(arvm_space_t space, arvm_function_t callee);

arvm_expr_t arvm_not(arvm_expr_t a);

arvm_expr_t arvm_make_ite(arvm_space_t space, arvm_expr_t a, arvm_expr_t b,
                          arvm_expr_t c);

arvm_expr_t arvm_make_and(arvm_space_t space, arvm_expr_t a, arvm_expr_t b);

arvm_expr_t arvm_make_or(arvm_space_t space, arvm_expr_t a, arvm_expr_t b);

arvm_expr_t arvm_make_xor(arvm_space_t space, arvm_expr_t a, arvm_expr_t b);

bool arvm_set_function_domain(arvm_space_t space, arvm_function_t func,
                              arvm_subdomain_t *domain);

void arvm_delete_function(arvm_space_t space, arvm_function_t func);

bool arvm_get_function_value(arvm_space_t space, arvm_function_t func);

#endif /* ARVM_H */