#include "match.h"
#include "analyze.h"
#include "arvm.h"
#include "ir.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void match_reset(pattern_t *pattern) {
  pattern->match = NULL;
  switch (pattern->kind) {
  case EXPR_NARY_FIXED:
  case EXPR_NARY:
    for (size_t i = 0; i < pattern->nary.operands.size; i++)
      match_reset(pattern->nary.operands.patterns[i]);
    break;
  default:
    break;
  }
}

bool match_next(pattern_t *pattern, arvm_expr_t expr) {
  switch (pattern->kind) {
  case EXPR_ANY:
  case EXPR_SLOT:
    if (pattern->match)
      return false;
    break;
  case EXPR_NARY_FIXED:
    if (!(expr->kind == NARY &&
          expr->nary.operands.size == pattern->nary.operands.size))
      return false;
  case EXPR_NARY: {
    if (!(expr->kind == NARY &&
          expr->nary.operands.size >= pattern->nary.operands.size &&
          val_matches(expr->nary.op, pattern->nary.op)))
      return false;
    bool initialized = true;
    if (!pattern->match) {
      initialized = false;
      for (size_t i = 0; i < pattern->nary.operands.size; i++)
        pattern->nary.cycles[i] = expr->nary.operands.size - i;
    }
    for (;;) {
      if (!initialized) {
        initialized = true;
        for (size_t i = 0; i < pattern->nary.operands.size; i++) {
          if (!match_next(pattern->nary.operands.patterns[i],
                          expr->nary.operands.exprs[i])) {
            goto nary_match_failure;
          }
        }
        goto nary_matched;
      } else {
        for (size_t i = 0; i < pattern->nary.operands.size; i++) {
          pattern_t *op_pattern = pattern->nary.operands.patterns[i];
          arvm_expr_t op_expr = expr->nary.operands.exprs[i];
          if (match_next(op_pattern, op_expr)) {
            goto nary_matched;
          } else {
            match_reset(op_pattern);
            match_next(op_pattern, op_expr);
          }
        }
      }

    nary_match_failure:
      initialized = false;
      for (size_t i = 0; i < pattern->nary.operands.size; i++)
        match_reset(pattern->nary.operands.patterns[i]);

      for (size_t i = pattern->nary.operands.size; i-- > 0;) {
        pattern->nary.cycles[i]--;
        arvm_expr_t operand = expr->nary.operands.exprs[i];
        if (pattern->nary.cycles[i] == 0) {
          memcpy(&expr->nary.operands.exprs[i],
                 &expr->nary.operands.exprs[i + 1],
                 (expr->nary.operands.size - i) * sizeof(arvm_expr_t));
          expr->nary.operands.exprs[expr->nary.operands.size - 1] = operand;
          pattern->nary.cycles[i] = expr->nary.operands.size - i;
        } else {
          size_t j = expr->nary.operands.size - pattern->nary.cycles[i];
          expr->nary.operands.exprs[i] = expr->nary.operands.exprs[j];
          expr->nary.operands.exprs[j] = operand;
          goto perm_next;
        }
      }
      break;
    perm_next:
      continue;
    }
    return false;
  nary_matched:
    break;
  }
  case EXPR_RANGE:
    if (pattern->match)
      return false;
    if (!(expr->kind == RANGE &&
          val_matches(expr->range.min, pattern->range.min) &&
          val_matches(expr->range.max, pattern->range.max)))
      return false;
    break;
  case EXPR_MODEQ:
    if (pattern->match)
      return false;
    if (!(expr->kind == MODEQ &&
          val_matches(expr->modeq.divisor, pattern->modeq.divisor) &&
          val_matches(expr->modeq.residue, pattern->modeq.residue)))
      return false;
    break;
  case EXPR_CALL:
    if (pattern->match)
      return false;
    if (!(expr->kind == CALL &&
          val_matches((arvm_val_t)expr->call.func, pattern->call.func) &&
          val_matches(expr->call.offset, pattern->call.offset)))
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
  case VAL_RANGE:
    if (!(val >= pattern->range.min && val <= pattern->range.max))
      return false;
    break;
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
      val_slots[*val_slot_count] = pattern;
    if (val_slot_count)
      (*val_slot_count)++;
  }
}

void find_slots(pattern_t *pattern, pattern_t **slots, size_t *slot_count,
                val_pattern_t **val_slots, size_t *val_slot_count) {
  if (slot_count == NULL) {
    size_t i = 0;
    slot_count = &i;
  }
  if (val_slot_count == NULL) {
    size_t i = 0;
    val_slot_count = &i;
  }
  switch (pattern->kind) {
  case EXPR_SLOT:
    if (slots)
      slots[*slot_count] = pattern;
    (*slot_count)++;
    break;
  case EXPR_NARY:
  case EXPR_NARY_FIXED:
    for (size_t i = 0; i < pattern->nary.operands.size; i++) {
      pattern_t *arg_pattern = pattern->nary.operands.patterns[i];
      find_slots(arg_pattern, slots, slot_count, val_slots, val_slot_count);
    }
    break;
  case EXPR_RANGE:
    find_val_slots(pattern->range.min, val_slots, val_slot_count);
    find_val_slots(pattern->range.max, val_slots, val_slot_count);
    break;
  case EXPR_MODEQ:
    find_val_slots(pattern->modeq.divisor, val_slots, val_slot_count);
    find_val_slots(pattern->modeq.residue, val_slots, val_slot_count);
    break;
  case EXPR_CALL:
    find_val_slots(pattern->call.func, val_slots, val_slot_count);
    find_val_slots(pattern->call.offset, val_slots, val_slot_count);
    break;
  default:
    break;
  }
}

bool matches(arvm_expr_t expr, pattern_t *pattern) {
  FOR_EACH_MATCH(expr, pattern, { return true; });
  return false;
}