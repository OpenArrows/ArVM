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
  case RANGE:
    return a->range.min == b->range.min && a->range.max == b->range.max;
  case MODEQ:
    return a->modeq.divisor == b->modeq.divisor &&
           a->modeq.residue == b->modeq.residue;
  case CALL:
    return a->call.func == b->call.func && a->call.offset == b->call.offset;
  case NONE:
  case UNKNOWN:
    return true;
  default:
    unreachable();
  }
}

bool arvm_has_calls(const arvm_expr_t expr) {
  switch (expr->kind) {
  case CALL:
    return true;
  case NARY:
    for (int i = 0; i < expr->nary.operands.size; i++)
      if (arvm_has_calls(expr->nary.operands.exprs[i]))
        return true;
  default:
    return false;
  }
}

bool arvm_ranges_overlap(const arvm_expr_t a, const arvm_expr_t b) {
  assert(a->kind == RANGE && b->kind == RANGE);
  return a->range.min <= b->range.max && b->range.min <= a->range.max;
}