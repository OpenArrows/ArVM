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
          match_next(pattern->in_interval.value, expr->in_interval.value) &&
          val_matches(expr->in_interval.min, pattern->in_interval.min) &&
          val_matches(expr->in_interval.max, pattern->in_interval.max)))
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

bool val_matches(arvm_val_t val, val_pattern_t *pattern) {
  switch (pattern->kind) {
  case VAL_ANY:
  case VAL_SLOT:
    break;
  case VAL_SPECIFIC:
    for (size_t i = 0; i < pattern->specific.values.size; i++)
      if (val == pattern->specific.values.vals[i])
        goto matched;
    return false;
  default:
    unreachable();
  }
matched:
  pattern->match = val;
  return true;
}

static void find_val_slots(val_pattern_t *pattern, val_pattern_t **val_slots,
                           size_t *val_slot_count) {
  if (pattern->kind == VAL_SLOT) {
    if (val_slots)
      *val_slots = pattern;
    if (val_slot_count)
      (*val_slot_count)++;
  }
}

static void find_slots(pattern_t *pattern, pattern_t **slots,
                       size_t *slot_count, val_pattern_t **val_slots,
                       size_t *val_slot_count) {
  switch (pattern->kind) {
  case EXPR_SLOT:
    if (slots)
      *slots = pattern;
    if (slot_count)
      (*slot_count)++;
    break;
  case EXPR_NARY:
  case EXPR_NARY_FIXED: {
    for (size_t i = 0; i < pattern->nary.args.size; i++) {
      pattern_t *arg_pattern = pattern->nary.args.patterns[i];
      size_t arg_slot_count = 0, arg_val_slot_count = 0;
      find_slots(arg_pattern, slots, &arg_slot_count, val_slots,
                 &arg_val_slot_count);
      if (slots)
        slots += arg_slot_count;
      if (slot_count)
        *slot_count += arg_slot_count;
      if (val_slots)
        val_slots += arg_val_slot_count;
      if (val_slot_count)
        *val_slot_count += arg_val_slot_count;
    }
    break;
  }
  case EXPR_IN_INTERVAL:
    find_slots(pattern->in_interval.value, slots, slot_count, val_slots,
               val_slot_count);
    find_val_slots(pattern->in_interval.min, val_slots, val_slot_count);
    find_val_slots(pattern->in_interval.max, val_slots, val_slot_count);
    break;
  case EXPR_CALL:
    find_slots(pattern->call.arg, slots, slot_count, val_slots, val_slot_count);
    break;
  case EXPR_CONST:
    find_val_slots(pattern->const_.value, val_slots, val_slot_count);
    break;
  default:
    break;
  }
}

bool matches(arvm_expr_t *expr, pattern_t *pattern) {
  match_init(pattern);

  size_t slot_count = 0, val_slot_count = 0;
  find_slots(pattern, NULL, &slot_count, NULL, &val_slot_count);
  if (slot_count == 0 && val_slot_count == 0)
    return match_next(pattern, expr);

  pattern_t *slots[slot_count];
  val_pattern_t *val_slots[val_slot_count];
  find_slots(pattern, slots, NULL, val_slots, NULL);

  while (match_next(pattern, expr)) {
    arvm_expr_t *match = slots[0]->match;
    for (size_t i = 0; i < slot_count; i++)
      if (!is_identical(match, slots[i]->match))
        goto skip;

    arvm_val_t val_match = val_slots[0]->match;
    for (size_t i = 0; i < val_slot_count; i++)
      if (val_slots[i]->match != val_match)
        goto skip;

    return true;
  skip:;
  }
  return false;
}