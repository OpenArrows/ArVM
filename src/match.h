#include "arvm.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum val_pattern_kind { VAL_ANY, VAL_SPECIFIC } val_pattern_kind_t;

typedef struct val_pattern {
  val_pattern_kind_t kind;
  union {
    struct {
      arvm_val_t value;
    } specific;
  };
} val_pattern_t;

typedef struct pattern pattern_t;

typedef struct pattern_list {
  size_t size;
  pattern_t **patterns;
} patternlist_t;

typedef enum pattern_kind {
  EXPR_ANY,
  EXPR_SLOT,
  EXPR_NARY,
  EXPR_NARY_FIXED,
  EXPR_NARY_EACH,
  EXPR_IN_INTERVAL,
  EXPR_ARG_REF,
  EXPR_CALL,
  EXPR_CONST
} pattern_kind_t;

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
      val_pattern_t op;
      patternlist_t args;
    } nary;
    struct {
      val_pattern_t op;
      pattern_t *arg;
    } nary_each;
    struct {
      pattern_t *value;
    } in_interval;
    struct {
      pattern_t *arg;
    } call;
    struct {
      val_pattern_t value;
    } const_;
  };
};

#define PATTERN_LIST(...)                                                      \
  ((patternlist_t){sizeof((pattern_t *[]){__VA_ARGS__}) / sizeof(pattern_t *), \
                   (pattern_t *[]){__VA_ARGS__}})

#define ANY_AS(capture) (&(pattern_t){EXPR_ANY, &capture})

#define ANY() ANY_AS(*NULL)

#define ANYVAL() ((val_pattern_t){VAL_ANY})

// TODO: #define SLOTVAL() (val_pattern_t) { VAL_SLOT }

#define VAL(val) ((val_pattern_t){VAL_SPECIFIC, .specific = val})

// Slots are the same as 'any' pattern, except that the expression in each slot
// must be identical to other matched expressions
#define SLOT_AS(capture) (&(pattern_t){EXPR_SLOT, &capture})

#define SLOT() SLOT_AS(*NULL)

// Matches an n-ary expression
#define NARY_AS(capture, op, ...)                                              \
  (&(pattern_t){EXPR_NARY, &capture,                                           \
                .nary = {op, .args = PATTERN_LIST(__VA_ARGS__)}})

#define NARY(op, ...) NARY_AS(*NULL, op, __VA_ARGS__)

// Matches an n-ary expression of fixed length
#define NARY_FIXED_AS(capture, op, ...)                                        \
  (&(pattern_t){EXPR_NARY_FIXED, &capture,                                     \
                .nary = {op, .args = PATTERN_LIST(__VA_ARGS__)}})

#define NARY_FIXED(op, ...) NARY_FIXED_AS(*NULL, op, __VA_ARGS__)

// Matches an n-ary expression only if each argument matches the given pattern
#define NARY_EACH_AS(capture, op, arg)                                         \
  (&(pattern_t){EXPR_NARY_EACH, &capture, .nary_each = {op, arg}})

#define NARY_EACH(op, arg) NARY_EACH_AS(*NULL, op, arg)

#define IN_INTERVAL_AS(capture, value)                                         \
  (&(pattern_t){EXPR_IN_INTERVAL, &capture, .in_interval = {value}})

#define IN_INTERVAL(value) IN_INTERVAL_AS(*NULL, value)

#define ARG_REF_AS(capture) (&(pattern_t){EXPR_ARG_REF, &capture})

#define ARG_REF() ARG_REF_AS(*NULL)

#define CALL_AS(capture, arg) (&(pattern_t){EXPR_CALL, &capture, .call = {arg}})

#define CALL(arg) CALL_AS(*NULL, arg)

#define CONST_AS(capture, value)                                               \
  (&(pattern_t){EXPR_CONST, &capture, .const_ = {value}})
#define CONST(...) CONST_AS(*NULL, ##__VA_ARGS__)

bool val_matches(arvm_val_t val, val_pattern_t pattern);

bool matches(arvm_expr_t *expr, pattern_t *pattern);