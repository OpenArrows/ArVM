#include "match.h"
#include "analyze.h"
#include <stddef.h>
#include <stdlib.h>

#define NARY_ARG_MATCH_BLOCK_SIZE 4

bool val_matches(arvm_val_t val, val_pattern_t pattern) {
  switch (pattern.kind) {
  case VAL_ANY:
    return true;
  case VAL_SPECIFIC:
    return val == pattern.specific.value;
  }
}

static bool try_match(arvm_expr_t *expr, pattern_t *pattern);

static bool cmp_pattern(arvm_expr_t *expr, pattern_t *pattern) {
  switch (pattern->kind) {
  case EXPR_ANY:
  case EXPR_SLOT:
    return true;
  case EXPR_NARY_FIXED:
    if (expr->kind != NARY || expr->nary.args.size != pattern->nary.args.size)
      return false;
  case EXPR_NARY:
    if (expr->kind != NARY || !val_matches(expr->nary.op, pattern->nary.op))
      return false;

    // TODO: set representatives algorithm
    for (int i = 0; i < pattern->nary.args.size; i++) {
      pattern_t *arg_pattern = pattern->nary.args.patterns[i];
      bool matched = false;
      for (int j = 0; j < expr->nary.args.size; j++) {
        arvm_expr_t *arg_expr = expr->nary.args.exprs[j];
        if (try_match(arg_expr, arg_pattern))
          matched = true;
      }
      if (!matched)
        return false;
    }
    return true;
  case EXPR_IN_INTERVAL:
    return expr->kind == IN_INTERVAL &&
           try_match(expr->in_interval.value, pattern->in_interval.value);
  case EXPR_ARG_REF:
    return expr->kind == ARG_REF;
  case EXPR_CALL:
    return expr->kind == CALL && try_match(expr->call.arg, pattern->call.arg);
  case EXPR_CONST:
    return expr->kind == CONST &&
           val_matches(expr->const_.value, pattern->const_.value);
  }
}

static bool try_match(arvm_expr_t *expr, pattern_t *pattern) {
  if (expr == NULL)
    return false;
  bool result = cmp_pattern(expr, pattern);
  if (result && pattern->capture)
    *pattern->capture = expr;
  return result;
}

static int find_slots(pattern_t *pattern, pattern_t **slots) {
  switch (pattern->kind) {
  case EXPR_SLOT:
    if (slots != NULL)
      *slots = pattern;
    return 1;
  case EXPR_NARY: {
    int sum = 0;
    for (int i = 0; i < pattern->nary.args.size; i++) {
      pattern_t *arg_pattern = pattern->nary.args.patterns[i];
      sum += find_slots(arg_pattern, slots++);
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
  bool result = try_match(expr, pattern);
  if (!result)
    return false;

  int slot_count = find_slots(pattern, NULL);
  if (slot_count == 0)
    return result;

  pattern_t *slots[slot_count];
  find_slots(pattern, slots);

  for (int i = 0; i < slot_count; i++) {
    if (slots[i]->slot.match_count == 0)
      return false;
  }

  int indices[slot_count];
  for (;;) {
    arvm_expr_t *expr = *slots[0]->capture = slots[0]->slot.matches[indices[0]];
    for (int i = 1; i < slot_count; i++) {
      if (!is_identical(expr, *slots[i]->capture =
                                  slots[i]->slot.matches[indices[i]]))
        goto skip;
    }

  skip:
    for (int i = 0; i < slot_count; i++) {
      indices[i]++;
      if (indices[i] > slots[i]->slot.match_count) {
        if (i == slot_count - 1)
          goto fail;
        indices[i] = 0;
      } else
        break;
    }
  }

fail:
  result = false;

end:
  for (int i = 0; i < slot_count; i++) {
    free(slots[i]->slot.matches);
    slots[i]->slot.matches = NULL;
  }

  return result;
}