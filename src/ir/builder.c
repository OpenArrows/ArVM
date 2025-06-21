#include "builder.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

arvm_expr_t arvm_new_expr(arvm_arena_t *arena, arvm_expr_kind_t kind) {
  arvm_expr_t expr = arvm_arena_alloc(arena, sizeof(struct arvm_expr));
  expr->kind = kind;
  return expr;
}

arvm_expr_t arvm_new_nary_v(arvm_arena_t *arena, arvm_nary_op_t op,
                            size_t operand_count, va_list operands) {
  arvm_expr_t expr = arvm_new_expr(arena, NARY);
  expr->nary.op = op;
  expr->nary.operands.size = operand_count;
  expr->nary.operands.exprs =
      arvm_arena_alloc(arena, sizeof(arvm_expr_t) * operand_count);
  for (int i = 0; i < operand_count; i++)
    expr->nary.operands.exprs[i] = va_arg(operands, arvm_expr_t);
  return expr;
}

arvm_expr_t arvm_new_nary(arvm_arena_t *arena, arvm_nary_op_t op,
                          size_t operand_count, ...) {
  va_list args;
  va_start(args, operand_count);
  arvm_expr_t nary = arvm_new_nary_v(arena, op, operand_count, args);
  va_end(args);
  return nary;
}

arvm_expr_t arvm_new_range(arvm_arena_t *arena, arvm_val_t min,
                           arvm_val_t max) {
  arvm_expr_t expr = arvm_new_expr(arena, RANGE);
  expr->range.min = min;
  expr->range.max = max;
  return expr;
}

arvm_expr_t arvm_new_modeq(arvm_arena_t *arena, arvm_val_t divisor,
                           arvm_val_t residue) {
  arvm_expr_t expr = arvm_new_expr(arena, MODEQ);
  expr->modeq.divisor = divisor;
  expr->modeq.residue = residue;
  return expr;
}

arvm_expr_t arvm_new_call(arvm_arena_t *arena, arvm_func_t func,
                          arvm_val_t offset) {
  arvm_expr_t expr = arvm_new_expr(arena, CALL);
  expr->call.func = func;
  expr->call.offset = offset;
  return expr;
}

arvm_expr_t arvm_clone(arvm_arena_t *arena, const arvm_expr_t expr) {
  arvm_expr_t clone = arvm_new_expr(arena, NONE);
  arvm_copy_expr(arena, expr, clone);
  return clone;
}

void arvm_copy_expr(arvm_arena_t *arena, const arvm_expr_t src,
                    arvm_expr_t dst) {
  dst->kind = src->kind;
  switch (src->kind) {
  case NARY:
    dst->nary.op = src->nary.op;
    dst->nary.operands.size = src->nary.operands.size;
    dst->nary.operands.exprs = arvm_arena_alloc(
        arena,
        sizeof(arvm_expr_t) * src->nary.operands.size); // TODO: reuse memory
    for (int i = 0; i < src->nary.operands.size; i++) {
      dst->nary.operands.exprs[i] =
          arvm_clone(arena, src->nary.operands.exprs[i]);
    }
    break;
  case RANGE:
    dst->range.min = src->range.min;
    dst->range.max = src->range.max;
    break;
  case MODEQ:
    dst->modeq.divisor = src->modeq.divisor;
    dst->modeq.residue = src->modeq.residue;
    break;
  case CALL:
    dst->call.func = src->call.func;
    dst->call.offset = src->call.offset;
    break;
  case NONE:
  case UNKNOWN:
    break;
  default:
    unreachable();
  }
}