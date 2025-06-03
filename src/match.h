#include "arvm.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum pattern_kind {
  EXPR_ANY,
  EXPR_SLOT,
  EXPR_NARY,
  EXPR_IN_INTERVAL,
  EXPR_ARG_REF,
  EXPR_CALL,
  EXPR_CONST,
  EXPR_CONSTVAL
} pattern_kind_t;

typedef struct pattern pattern_t;

typedef struct pattern_list {
  size_t size;
  pattern_t *patterns;
} patternlist_t;

struct pattern {
  pattern_kind_t kind;
  arvm_expr_t **capture;
  union {
    struct {
      size_t capacity;
      size_t match_count;
      arvm_expr_t **matches;
    } slot;
    struct {
      arvm_nary_op_t op;
      patternlist_t args;
    } nary;
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
};

#define PATTERN_LIST(...)                                                      \
  (patternlist_t) {                                                            \
    sizeof((pattern_t[]){__VA_ARGS__}) / sizeof(pattern_t), (pattern_t[]) {    \
      __VA_ARGS__                                                              \
    }                                                                          \
  }

#define ANY_AS(capture)                                                        \
  &(pattern_t) { EXPR_ANY, &capture }

#define ANY() ANY_AS(*NULL)

// Slots are the same as 'any' pattern, except that the expression in each slot
// must be identical to other matched expressions
#define SLOT_AS(capture, index)                                                \
  &(pattern_t) {                                                               \
    EXPR_SLOT, &capture, .slot = { index }                                     \
  }

#define SLOT(index) SLOT_AS(*NULL, index)

#define NARY_AS(capture, op, ...)                                              \
  &(pattern_t) {                                                               \
    EXPR_NARY, &capture, .nary = { op, PATTERN_LIST(__VA_ARGS__) }             \
  }

#define NARY(op, ...) NARY_AS(*NULL, __VA_ARGS__)

#define IN_INTERVAL_AS(capture, value)                                         \
  &(pattern_t) {                                                               \
    EXPR_IN_INTERVAL, &capture, .in_interval = { value }                       \
  }

#define IN_INTERVAL(value) IN_INTERVAL_AS(*NULL, value)

#define ARG_REF_AS(capture)                                                    \
  &(pattern_t) { EXPR_ARG_REF, &capture }

#define ARG_REF() ARG_REF_AS(*NULL)

#define CALL_AS(capture, arg)                                                  \
  &(pattern_t) {                                                               \
    EXPR_CALL, &capture, .call = { arg }                                       \
  }

#define CALL(arg) CALL_AS(*NULL, arg)

#define CONST_ANY_AS(capture)                                                  \
  &(pattern_t) { EXPR_CONST, &capture }
#define CONST_VAL_AS(capture, value)                                           \
  &(pattern_t) {                                                               \
    EXPR_CONSTVAL, &capture, .constval = { value }                             \
  }

#define GET_CONST_AS_MACRO(_1, _2, macro, ...) macro
#define CONST_AS(...)                                                          \
  GET_CONST_AS_MACRO(, ##__VA_ARGS__, CONST_VAL, CONST_ANY)(__VA_ARGS__)

#define CONST(...) CONST_AS(*NULL, ##__VA_ARGS__)

bool matches(arvm_expr_t *expr, pattern_t *pattern);