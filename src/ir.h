#ifndef IR_H
#define IR_H

#include "arvm.h"
#include <stddef.h>

typedef struct arvm_exprlist {
  size_t size;
  arvm_expr_t *exprs;
} arvm_exprlist_t;

// Binary expression, equivalent to non-commutative operations
typedef struct arvm_binary_expr {
  arvm_binary_op_t op;
  arvm_expr_t lhs;
  arvm_expr_t rhs;
} arvm_binary_expr_t;

// N-ary expression, equivalent to commutative arithmetic/boolean operations
typedef struct arvm_nary_expr {
  arvm_nary_op_t op;
  arvm_exprlist_t operands;
} arvm_nary_expr_t;

// Interval expression, checks whether the value is within an expected range
// (inclusive)
typedef struct arvm_in_interval_expr {
  arvm_expr_t value;
  arvm_val_t min;
  arvm_val_t max;
} arvm_in_interval_expr_t;

// Call expression
typedef struct arvm_call_expr {
  arvm_func_t func;
  arvm_expr_t arg;
} arvm_call_expr_t;

// Constant expression, used to represent integer values
typedef struct arvm_const_expr {
  arvm_val_t value;
} arvm_const_expr_t;

typedef enum arvm_expr_kind {
  BINARY,
  NARY,
  IN_INTERVAL,
  ARG_REF,
  CALL,
  CONST,

  // Special expression kinds
  NONE = -1,
  UNKNOWN = -2,
  RESERVED = -3
} arvm_expr_kind_t;

struct arvm_expr {
  arvm_expr_kind_t kind;
  union {
    arvm_binary_expr_t binary;
    arvm_nary_expr_t nary;
    arvm_in_interval_expr_t in_interval;
    arvm_call_expr_t call;
    arvm_const_expr_t const_;
  };
};

// Represents an unary function f(t), t > 0, that returns a boolean value
// (active/inactive state) for each tick
struct arvm_func {
  // Expression that defines the function
  arvm_expr_t value;

  // Pointer to a JIT-compiled function
  arvm_val_t (*func)(arvm_val_t);
};

#endif /* IR_H */