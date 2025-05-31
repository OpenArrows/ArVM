#include "arvm.h"
#include <stdbool.h>

typedef enum pattern_kind {
  EXPR_ANY,
  EXPR_BINARY,
  EXPR_IN_INTERVAL,
  EXPR_ARG_REF,
  EXPR_CALL,
  EXPR_CONST,
  EXPR_CONSTVAL
} pattern_kind_t;

typedef struct pattern {
  pattern_kind_t kind;
  arvm_expr_t **capture;
  union {
    struct {
      arvm_binop_t op;
      pattern_t *lhs;
      pattern_t *rhs;
    } binary;
    struct {
      pattern_t *value;
    } in_interval;
    struct {
      pattern_t *arg;
    } call;
    struct {
      arvm_val_t value;
    } constval;
  };
} pattern_t;

#define ANY(...)                                                               \
  &(pattern_t) { EXPR_ANY, ##__VA_ARGS__ }

#define BINARY(op, lhs, rhs, ...)                                              \
  &(pattern_t) {                                                               \
    EXPR_BINARY, ##__VA_ARGS__, .binary = { op, lhs, rhs }                     \
  }

#define IN_INTERVAL(value, ...)                                                \
  &(pattern_t) {                                                               \
    EXPR_IN_INTERVAL, ##__VA_ARGS__, .in_interval = { value }                  \
  }

#define ARG_REF(...)                                                           \
  &(pattern_t) { EXPR_ARG_REF, ##__VA_ARGS__ }

#define CALL(arg, ...)                                                         \
  &(pattern_t) {                                                               \
    EXPR_CALL, ##__VA_ARGS__, .call = { arg }                                  \
  }

#define CONST(...)                                                             \
  &(pattern_t) { EXPR_CONST, ##__VA_ARGS__ }

#define CONSTVAL(value, ...)                                                   \
  &(pattern_t) {                                                               \
    EXPR_CONSTVAL, ##__VA_ARGS__, .constval = { value }                        \
  }

bool matches(arvm_expr_t *expr, pattern_t *pattern);