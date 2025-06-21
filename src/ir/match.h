#ifndef MATCH_H
#define MATCH_H

#include "arvm.h"
#include "util/macros.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct val_list {
  size_t size;
  arvm_val_t *vals;
} vallist_t;

typedef enum val_pattern_kind {
  VAL_ANY,
  VAL_SLOT,
  VAL_SPECIFIC,
  VAL_RANGE
} val_pattern_kind_t;

typedef struct val_pattern {
  val_pattern_kind_t kind;
  arvm_val_t match;
  union {
    struct {
      vallist_t values;
    } specific;
    struct {
      arvm_val_t min;
      arvm_val_t max;
    } range;
  };
} val_pattern_t;

#define VAL_LIST(...)                                                          \
  ((vallist_t){(lengthof(((arvm_val_t[]){__VA_ARGS__}))),                      \
               (arvm_val_t[]){__VA_ARGS__}})

#define ANYVAL() (&(val_pattern_t){VAL_ANY})

#define SLOTVAL() (&(val_pattern_t){VAL_SLOT})

#define VAL(...)                                                               \
  (&(val_pattern_t){VAL_SPECIFIC, .specific = VAL_LIST(__VA_ARGS__)})

#define RANGEVAL(min, max) (&(val_pattern_t){VAL_RANGE, .range = {min, max}})

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
  EXPR_RANGE,
  EXPR_MODEQ,
  EXPR_CALL
} pattern_kind_t;

struct pattern {
  pattern_kind_t kind;
  arvm_expr_t *capture;
  arvm_expr_t match;
  union {
    struct {
      val_pattern_t *op;
      patternlist_t operands;
      size_t *cycles;
    } nary;
    struct {
      val_pattern_t *min;
      val_pattern_t *max;
    } range;
    struct {
      val_pattern_t *divisor;
      val_pattern_t *residue;
    } modeq;
    struct {
      val_pattern_t *func;
      val_pattern_t *offset;
    } call;
  };
};

#define PATTERN_LIST(...)                                                      \
  ((patternlist_t){lengthof(((pattern_t *[]){__VA_ARGS__})),                   \
                   (pattern_t *[]){__VA_ARGS__}})

#define ANY_AS(capture) (&(pattern_t){EXPR_ANY, &capture})

#define ANY() ANY_AS(*NULL)

// Slots are the same as 'any' pattern, except that the expression in each slot
// must be identical to other matched expressions
#define SLOT_AS(capture) (&(pattern_t){EXPR_SLOT, &capture})

#define SLOT() SLOT_AS(*NULL)

// Matches an n-ary expression
#define NARY_AS(capture, op, ...)                                              \
  (&(pattern_t){                                                               \
      EXPR_NARY, &capture,                                                     \
      .nary = {op, .operands = PATTERN_LIST(__VA_ARGS__),                      \
               .cycles =                                                       \
                   (size_t[lengthof(((pattern_t *[]){__VA_ARGS__}))]){0}}})

#define NARY(op, ...) NARY_AS(*NULL, op, __VA_ARGS__)

// Matches an n-ary expression of fixed length
#define NARY_FIXED_AS(capture, op, ...)                                        \
  (&(pattern_t){                                                               \
      EXPR_NARY_FIXED, &capture,                                               \
      .nary = {op, .operands = PATTERN_LIST(__VA_ARGS__),                      \
               .cycles =                                                       \
                   (size_t[lengthof(((pattern_t *[]){__VA_ARGS__}))]){0}}})

#define NARY_FIXED(op, ...) NARY_FIXED_AS(*NULL, op, __VA_ARGS__)

#define RANGE_AS(capture, min, max)                                            \
  (&(pattern_t){EXPR_RANGE, &capture, .range = {min, max}})

#define RANGE(min, max) RANGE_AS(*NULL, min, max)

#define MODEQ_AS(capture, divisor, residue)                                    \
  (&(pattern_t){EXPR_MODEQ, &capture, .modeq = {divisor, residue}})

#define MODEQ(divisor, residue) MODEQ_AS(*NULL, divisor, residue)

#define CALL_AS(capture, func, offset)                                         \
  (&(pattern_t){EXPR_CALL, &capture, .call = {func, offset}})

#define CALL(func, offset) CALL_AS(*NULL, func, offset)

bool val_matches(arvm_val_t val, val_pattern_t *pattern);

void match_reset(pattern_t *pattern);

bool match_next(pattern_t *pattern, arvm_expr_t expr);

void find_slots(pattern_t *pattern, pattern_t **slots, size_t *slot_count,
                val_pattern_t **val_slots, size_t *val_slot_count);

bool matches(arvm_expr_t expr, pattern_t *pattern);

#define FOR_EACH_MATCH(expr, pattern, block)                                   \
  do {                                                                         \
    pattern_t *_pattern = pattern;                                             \
    match_reset(_pattern);                                                     \
                                                                               \
    size_t _slot_count = 0, _val_slot_count = 0;                               \
    find_slots(_pattern, NULL, &_slot_count, NULL, &_val_slot_count);          \
                                                                               \
    pattern_t *_slots[_slot_count];                                            \
    val_pattern_t *_val_slots[_val_slot_count];                                \
    find_slots(_pattern, _slots, NULL, _val_slots, NULL);                      \
                                                                               \
    while (match_next(_pattern, expr)) {                                       \
      if (_slot_count > 0) {                                                   \
        arvm_expr_t match = _slots[0]->match;                                  \
        for (size_t i = 0; i < _slot_count; i++)                               \
          if (!arvm_is_identical(match, _slots[i]->match))                     \
            goto UNIQUE(_skip);                                                \
      }                                                                        \
                                                                               \
      if (_val_slot_count > 0) {                                               \
        arvm_val_t val_match = _val_slots[0]->match;                           \
        for (size_t i = 0; i < _val_slot_count; i++)                           \
          if (_val_slots[i]->match != val_match)                               \
            goto UNIQUE(_skip);                                                \
      }                                                                        \
                                                                               \
      block;                                                                   \
                                                                               \
      UNIQUE(_skip) :;                                                         \
    }                                                                          \
  } while (0);

#endif /* MATCH_H */