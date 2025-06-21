#ifndef IR_H
#define IR_H

#include "arvm.h"
#include <stddef.h>

typedef struct arvm_exprlist {
  size_t size;
  arvm_expr_t *exprs;
} arvm_exprlist_t;

typedef enum arvm_expr_kind {
  RANGE,
  MODEQ,
  NARY,
  CALL,

  // Special expression kinds
  // TODO: remove
  NONE = -1,
  UNKNOWN = -2,
  RESERVED = -3
} arvm_expr_kind_t;

struct arvm_expr {
  arvm_expr_kind_t kind;
  union {
    // Range expression, checks whether the argument value is within an expected
    // range (inclusive)
    struct {
      arvm_val_t min;
      arvm_val_t max;
    } range;

    // Modulo equality expression, checks whether the argument's residue when
    // divided by given divisor is equal to the expected value
    struct {
      arvm_val_t divisor;
      arvm_val_t residue;
    } modeq;

    // N-ary expression, represents commutative arithmetic/boolean operations
    struct {
      arvm_nary_op_t op;
      arvm_exprlist_t operands;
    } nary;

    // Call expression
    struct {
      arvm_func_t func;
      arvm_val_t offset;
    } call;
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