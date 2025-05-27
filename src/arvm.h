#include <limits.h>
#include <stdint.h>

#ifndef ARVM
#define ARVM
typedef long long arvm_val_t;

typedef struct expression arvm_expr_t;

typedef struct function arvm_func_t;

typedef enum binary_op { OR, AND, XOR, ADD, MOD } arvm_binop_t;

// Binary expression, equivalent to arithmetic/boolean operations
typedef struct binary_expr {
  arvm_binop_t op;
  arvm_expr_t *lhs;
  arvm_expr_t *rhs;
} arvm_binary_expr_t;

// Interval expression, checks whether the value is within an expected range
// (inclusive)
typedef struct in_interval_expr {
  arvm_expr_t *value;
  arvm_val_t min;
  arvm_val_t max;
} arvm_in_interval_expr_t;

typedef enum ref_type { ARG } arvm_ref_t;

// Reference expression, can be used to retrieve the argument value
typedef struct ref_expr {
  arvm_ref_t ref;
} arvm_ref_expr_t;

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
  BINARY,
  IN_INTERVAL,
  REF,
  CALL,
  CONST,
  NONE = -1
} arvm_expr_kind_t;

struct expression {
  arvm_expr_kind_t kind;
  union {
    arvm_binary_expr_t binary;
    arvm_in_interval_expr_t in_interval;
    arvm_ref_expr_t ref;
    arvm_call_expr_t call;
    arvm_const_expr_t const_;
  };
};

struct function {
  arvm_expr_t *value;
};

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_NEGATIVE_INFINITY LLONG_MIN
#define ARVM_POSITIVE_INFINITY LLONG_MAX
#endif