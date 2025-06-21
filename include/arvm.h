#ifndef ARVM_H
#define ARVM_H

#include <stdint.h>

typedef uintmax_t arvm_val_t;

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_INFINITY UINTMAX_MAX

typedef struct arvm_expr *arvm_expr_t;

typedef struct arvm_func *arvm_func_t;

typedef struct arvm_ctx *arvm_ctx_t;

arvm_ctx_t arvm_create_context();

void arvm_release_context(arvm_ctx_t ctx);

arvm_func_t arvm_create_function(arvm_ctx_t ctx, arvm_expr_t value);

void arvm_finalize(arvm_ctx_t ctx);

arvm_val_t arvm_call_function(arvm_func_t func, arvm_val_t arg);

typedef enum arvm_nary_op {
  ARVM_OP_OR,
  ARVM_OP_NOR,
  ARVM_OP_XOR,
  ARVM_OP_TH2, // TH2 (2-threshold) is true if at least two of the given
               // operands are true
} arvm_nary_op_t;

// Boolean n-ary expression
arvm_expr_t arvm_make_nary(arvm_ctx_t ctx, arvm_nary_op_t op,
                           size_t operand_count, ...);

// Checks if the argument is in given range
arvm_expr_t arvm_make_range(arvm_ctx_t ctx, arvm_val_t min, arvm_val_t max);

// Checks if the argument's residue is equal to the given value when divided by
// given constant divisor
arvm_expr_t arvm_make_modeq(arvm_ctx_t ctx, arvm_val_t divisor,
                            arvm_val_t residue);

// Evaluates another ArVM function with the current argument value - constant
// offset
arvm_expr_t arvm_make_call(arvm_ctx_t ctx, arvm_func_t func, arvm_val_t offset);

#endif /* ARVM_H */