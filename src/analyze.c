#include "analyze.h"
#include "ir.h"
#include <assert.h>
#include <string.h>

bool arvm_is_identical(const arvm_expr_t a, const arvm_expr_t b) {
  if (a == NULL || b == NULL)
    return false;

  if (a->kind != b->kind)
    return false;

  switch (a->kind) {
  case BINARY:
    return a->binary.op == b->binary.op &&
           arvm_is_identical(a->binary.lhs, b->binary.lhs) &&
           arvm_is_identical(a->binary.rhs, b->binary.rhs);
  case NARY: {
    if (a->nary.op != b->nary.op)
      return false;

    size_t operand_count = a->nary.operands.size;
    if (b->nary.operands.size != operand_count)
      return false;

    arvm_expr_t operands[operand_count];
    memcpy(operands, a->nary.operands.exprs, sizeof(operands));
    for (int i = 0; i < operand_count; i++) {
      arvm_expr_t operands_b = b->nary.operands.exprs[i];
      for (int j = 0; j < operand_count; j++) {
        if (arvm_is_identical(operands[j], operands_b)) {
          operands[j] = NULL;
          goto next;
        }
      }
      return false;
    next:
      continue;
    }
    return true;
  }
  case IN_INTERVAL:
    return a->in_interval.min == b->in_interval.min &&
           a->in_interval.max == b->in_interval.max &&
           arvm_is_identical(a->in_interval.value, b->in_interval.value);
  case CALL:
    return a->call.func == b->call.func &&
           arvm_is_identical(a->call.arg, b->call.arg);
  case CONST:
    return a->const_.value == b->const_.value;
  case ARG_REF:
  case NONE:
  case UNKNOWN:
    return true;
  default:
    unreachable();
  }
}

bool arvm_has_calls(const arvm_expr_t expr) {
  switch (expr->kind) {
  case BINARY:
    return arvm_has_calls(expr->binary.lhs) || arvm_has_calls(expr->binary.rhs);
  case NARY:
    for (int i = 0; i < expr->nary.operands.size; i++)
      if (arvm_has_calls(expr->nary.operands.exprs[i]))
        return true;
    return false;
  case IN_INTERVAL:
    return arvm_has_calls(expr->in_interval.value);
  case CALL:
    return true;
  case CONST:
  case ARG_REF:
  case NONE:
  case UNKNOWN:
    return false;
  default:
    unreachable();
  }
}

bool arvm_intervals_overlap(const arvm_expr_t a, const arvm_expr_t b) {
  assert(a->kind == IN_INTERVAL && b->kind == IN_INTERVAL);
  return a->in_interval.min <= b->in_interval.max &&
         b->in_interval.min <= a->in_interval.max;
}