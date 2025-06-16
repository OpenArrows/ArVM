#ifndef ARVM_H
#define ARVM_H

#include <stdint.h>

typedef intmax_t arvm_val_t;

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_NEGATIVE_INFINITY INTMAX_MIN
#define ARVM_POSITIVE_INFINITY INTMAX_MAX

typedef struct arvm_expr *arvm_expr_t;

typedef struct arvm_func *arvm_func_t;

typedef struct arvm_ctx *arvm_ctx_t;

arvm_ctx_t arvm_create_context();

void arvm_release_context(arvm_ctx_t ctx);

arvm_func_t arvm_create_function(arvm_ctx_t ctx, arvm_expr_t value);

void arvm_finalize(arvm_ctx_t ctx);

arvm_val_t arvm_call_function(arvm_func_t func, arvm_val_t arg);

typedef enum arvm_binary_op { ARVM_BINARY_MOD } arvm_binary_op_t;

typedef enum arvm_nary_op {
  ARVM_NARY_OR,
  ARVM_NARY_AND,
  ARVM_NARY_XOR,
  ARVM_NARY_ADD
} arvm_nary_op_t;

arvm_expr_t arvm_make_binary(arvm_ctx_t ctx, arvm_binary_op_t op,
                             arvm_expr_t lhs, arvm_expr_t rhs);

arvm_expr_t arvm_make_nary(arvm_ctx_t ctx, arvm_nary_op_t op,
                           size_t operand_count, ...);

arvm_expr_t arvm_make_in_interval(arvm_ctx_t ctx, arvm_expr_t value,
                                  arvm_val_t min, arvm_val_t max);

arvm_expr_t arvm_make_arg_ref(arvm_ctx_t ctx);

arvm_expr_t arvm_make_call(arvm_ctx_t ctx, arvm_func_t func, arvm_expr_t arg);

arvm_expr_t arvm_make_const(arvm_ctx_t ctx, arvm_val_t value);

#endif /* ARVM_H */