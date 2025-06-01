#include "match.h"
#include "analyze.h"
#include <stddef.h>

static bool cmp_pattern(arvm_expr_t *expr, pattern_t *pattern) {
  switch (pattern->kind) {
  case EXPR_ANY:
    return true;
  case EXPR_BINARY: {
    if (expr->kind != BINARY || expr->binary.op != pattern->binary.op)
      return false;

    arvm_expr_t *lhs, **lhs_src, *rhs, **rhs_src;

    // If the binary operation is symmetric, we should check both sides
    // recursively
    if (is_symmetric(pattern->binary.op)) {
      pattern_t lhs_pattern;
      lhs_pattern = *pattern->binary.lhs;
      lhs_pattern.capture = &lhs;

      pattern_t rhs_pattern;
      rhs_pattern = *pattern->binary.rhs;
      rhs_pattern.capture = &rhs;

      arvm_expr_t *subexpr;
      if (matches(expr->binary.lhs,
                  BINARY(pattern->binary.op, &lhs_pattern, ANY(), &subexpr))) {
        lhs_src = &subexpr->binary.lhs;
      } else if (matches(expr->binary.lhs, BINARY(pattern->binary.op, ANY(),
                                                  &rhs_pattern, &subexpr))) {
        rhs_src = &subexpr->binary.lhs;
      }
      if (matches(expr->binary.rhs,
                  BINARY(pattern->binary.op, &lhs_pattern, ANY(), &subexpr))) {
        lhs_src = &subexpr->binary.rhs;
      } else if (matches(expr->binary.rhs, BINARY(pattern->binary.op, ANY(),
                                                  &rhs_pattern, &subexpr))) {
        rhs_src = &subexpr->binary.rhs;
      }
      if (!rhs || !lhs) {
        lhs = expr->binary.lhs;
        lhs_src = NULL;
        rhs = expr->binary.rhs;
        rhs_src = NULL;
      }
      if (matches(lhs, pattern->binary.rhs) &&
          matches(rhs, pattern->binary.lhs)) {
        arvm_expr_t *tmp = lhs, **tmp_src = lhs_src;
        lhs = rhs;
        lhs_src = rhs_src;
        rhs = tmp;
        rhs = tmp_src;
      }
      bool result = matches(lhs, pattern->binary.lhs) &&
                    matches(rhs, pattern->binary.rhs);
      if (result) {
        if (lhs_src)
          *lhs_src = expr->binary.lhs;
        expr->binary.lhs = lhs;
        if (rhs_src)
          *rhs_src = expr->binary.lhs;
        expr->binary.lhs = rhs;
      }
      return result;
    } else
      return matches(expr->binary.lhs, pattern->binary.lhs) &&
             matches(expr->binary.rhs, pattern->binary.rhs);
  }
  case EXPR_IN_INTERVAL:
    return expr->kind == IN_INTERVAL &&
           matches(expr->in_interval.value, pattern->in_interval.value);
  case EXPR_ARG_REF:
    return expr->kind == REF && expr->ref.ref == ARG;
  case EXPR_CALL:
    return expr->kind == CALL && matches(expr->call.arg, pattern->call.arg);
  case EXPR_CONST:
    return expr->kind == CONST;
  case EXPR_CONSTVAL:
    return expr->kind == CONST && expr->const_.value == pattern->constval.value;
  }
}

bool matches(arvm_expr_t *expr, pattern_t *pattern) {
  if (expr == NULL)
    return false;
  bool result = cmp_pattern(expr, pattern);
  if (result)
    *pattern->capture = expr;
  return result;
}