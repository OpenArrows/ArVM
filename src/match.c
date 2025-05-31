#include "match.h"
#include "analyze.h"

static bool cmp_pattern(arvm_expr_t *expr, pattern_t *pattern) {
  switch (pattern->kind) {
  case EXPR_ANY:
    return true;
  case EXPR_BINARY:
    if (expr->kind != BINARY || expr->binary.op != pattern->binary.op)
      return false;
    if (is_symmetric(pattern->binary.op))
      return (matches(expr->binary.lhs, pattern->binary.lhs) ||
              matches(expr->binary.lhs,
                      BINARY(pattern->binary.op, pattern->binary.lhs,
                             ANY(pattern->binary.rhs->capture))) ||
              matches(expr->binary.rhs, pattern->binary.lhs) ||
              matches(expr->binary.rhs,
                      BINARY(pattern->binary.op, pattern->binary.lhs,
                             ANY(pattern->binary.rhs->capture)))) &&
             (matches(expr->binary.lhs, pattern->binary.rhs) ||
              matches(expr->binary.lhs,
                      BINARY(pattern->binary.op, pattern->binary.rhs,
                             ANY(pattern->binary.rhs->capture))) ||
              matches(expr->binary.rhs, pattern->binary.rhs) ||
              matches(expr->binary.rhs,
                      BINARY(pattern->binary.op, pattern->binary.rhs,
                             ANY(pattern->binary.rhs->capture))));
    else
      return matches(expr->binary.lhs, pattern->binary.lhs) &&
             matches(expr->binary.rhs, pattern->binary.rhs);
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
  bool result = cmp_pattern(expr, pattern);
  if (result)
    *pattern->capture = expr;
  return result;
}