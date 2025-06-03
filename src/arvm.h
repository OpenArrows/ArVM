#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifndef ARVM
#define ARVM
typedef long long arvm_val_t;

typedef struct expression arvm_expr_t;

typedef struct expression_list {
  size_t size;
  arvm_expr_t **exprs;
} arvm_exprlist_t;

typedef struct function arvm_func_t;

typedef enum nary_op { OR, AND, XOR, ADD } arvm_nary_op_t;

// Binary expression, equivalent to arithmetic/boolean operations
typedef struct nary_expr {
  arvm_nary_op_t op;
  arvm_exprlist_t args;
} arvm_nary_expr_t;

// Interval expression, checks whether the value is within an expected range
// (inclusive)
typedef struct in_interval_expr {
  arvm_expr_t *value;
  arvm_val_t min;
  arvm_val_t max;
} arvm_in_interval_expr_t;

// Call expression
typedef struct call_expr {
  arvm_func_t *target;
  arvm_expr_t *arg;
} arvm_call_expr_t;

// Constant expression, used to represent integer values
typedef struct const_expr {
  arvm_val_t value;
} arvm_const_expr_t;

typedef enum expr_kind {
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

struct expression {
  arvm_expr_kind_t kind;
  union {
    arvm_nary_expr_t nary;
    arvm_in_interval_expr_t in_interval;
    arvm_call_expr_t call;
    arvm_const_expr_t const_;
  };
};

// Represents an unary function f(t), t > 0, that returns a boolean value
// (active/inactive state) for each tick
struct function {
  arvm_expr_t *value;
};

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_NEGATIVE_INFINITY LLONG_MIN
#define ARVM_POSITIVE_INFINITY LLONG_MAX
#endif