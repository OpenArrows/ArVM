#include "match.h"
#include "analyze.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void match_init(pattern_t *pattern) {
  pattern->match = NULL;
  switch (pattern->kind) {
  case EXPR_NARY_FIXED:
  case EXPR_NARY:
    free(pattern->nary.perm_it.cycles);
    pattern->nary.perm_it.cycles = NULL;
    for (size_t i = 0; i < pattern->nary.args.size; i++)
      match_init(pattern->nary.args.patterns[i]);
    break;
  case EXPR_NARY_EACH:
    match_init(pattern->nary_each.arg);
    break;
  case EXPR_IN_INTERVAL:
    match_init(pattern->in_interval.value);
    break;
  case EXPR_CALL:
    match_init(pattern->call.arg);
    break;
  default:
    break;
  }
}

bool match_next(pattern_t *pattern, arvm_expr_t *expr) {
  switch (pattern->kind) {
  case EXPR_ANY:
  case EXPR_SLOT:
    if (pattern->match)
      return false;
    break;
  case EXPR_NARY_FIXED:
    if (!(expr->kind == NARY &&
          expr->nary.args.size == pattern->nary.args.size))
      return false;
  case EXPR_NARY: {
    if (!(expr->kind == NARY &&
          expr->nary.args.size >= pattern->nary.args.size &&
          val_matches(expr->nary.op, pattern->nary.op)))
      return false;
    do {
      if (!pattern->match) {
        for (size_t i = 0; i < pattern->nary.args.size; i++)
          if (!match_next(pattern->nary.args.patterns[i],
                          expr->nary.args.exprs[i]))
            goto nary_match_failure;
        goto nary_matched;
      } else {
        for (size_t i = 0; i < pattern->nary.args.size; i++)
          if (match_next(pattern->nary.args.patterns[i],
                         expr->nary.args.exprs[i])) {
            goto nary_matched;
          } else {
            match_init(pattern->nary.args.patterns[i]);
            match_next(pattern->nary.args.patterns[i],
                       expr->nary.args.exprs[i]);
          }
      nary_match_failure:
        pattern->nary.perm_it.array = expr->nary.args.exprs;
        pattern->nary.perm_it.length = expr->nary.args.size;
        pattern->nary.perm_it.permutation_length = pattern->nary.args.size;
        for (size_t i = 0; i < pattern->nary.args.size; i++)
          match_init(pattern->nary.args.patterns[i]);
      }
    } while (permutation(&pattern->nary.perm_it));
    return false;
  nary_matched:
    break;
  }
  case EXPR_IN_INTERVAL:
    if (!(expr->kind == IN_INTERVAL &&
          match_next(pattern->in_interval.value, expr->in_interval.value)))
      return false;
    break;
  case EXPR_ARG_REF:
    if (expr->kind != ARG_REF)
      return false;
    break;
  case EXPR_CALL:
    if (!(expr->kind == CALL && match_next(pattern->call.arg, expr->call.arg)))
      return false;
    break;
  case EXPR_CONST:
    if (pattern->match)
      return false;
    if (!(expr->kind == CONST &&
          val_matches(expr->const_.value, pattern->const_.value)))
      return false;
    break;
  default:
    unreachable();
  }
  pattern->match = expr;
  if (pattern->capture)
    *pattern->capture = expr;
  return true;
}

bool val_matches(arvm_val_t val, val_pattern_t pattern) {
  switch (pattern.kind) {
  case VAL_ANY:
    return true;
  case VAL_SPECIFIC:
    return val == pattern.specific.value;
  default:
    unreachable();
  }
}

static size_t find_slots(pattern_t *pattern, pattern_t **slots) {
  switch (pattern->kind) {
  case EXPR_SLOT:
    if (slots)
      *slots = pattern;
    return 1;
  case EXPR_NARY: {
    size_t sum = 0;
    for (size_t i = 0; i < pattern->nary.args.size; i++) {
      pattern_t *arg_pattern = pattern->nary.args.patterns[i];
      sum += find_slots(arg_pattern, slots == NULL ? NULL : slots++);
    }
    return sum;
  }
  case EXPR_IN_INTERVAL:
    return find_slots(pattern->in_interval.value, slots);
  case EXPR_CALL:
    return find_slots(pattern->call.arg, slots);
  default:
    return 0;
  }
}

bool matches(arvm_expr_t *expr, pattern_t *pattern) {
  match_init(pattern);

  size_t slot_count = find_slots(pattern, NULL);
  if (slot_count == 0)
    return match_next(pattern, expr);

  pattern_t *slots[slot_count];
  find_slots(pattern, slots);

  while (match_next(pattern, expr)) {
    arvm_expr_t *match = slots[0]->match;
    for (size_t i = 0; i < slot_count; i++)
      if (!is_identical(match, slots[i]->match))
        goto skip;
    return true;
  skip:;
  }
  return false;
}