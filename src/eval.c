#include "eval.h"
#include "ir/ir.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct eval_ctx {
  arvm_val_t arg;
} eval_ctx_t;

static arvm_val_t eval_expr(arvm_expr_t expr, eval_ctx_t ctx) {
  switch (expr->kind) {
  case NARY:
    switch (expr->nary.op) {
    case ARVM_OP_OR:
      for (size_t i = 0; i < expr->nary.operands.size; i++) {
        if (eval_expr(expr->nary.operands.exprs[i], ctx))
          return ARVM_TRUE;
      }
      return ARVM_FALSE;
    case ARVM_OP_NOR:
      for (size_t i = 0; i < expr->nary.operands.size; i++) {
        if (eval_expr(expr->nary.operands.exprs[i], ctx))
          return ARVM_FALSE;
      }
      return ARVM_TRUE;
    case ARVM_OP_XOR: {
      arvm_val_t accum = ARVM_FALSE;
      for (size_t i = 0; i < expr->nary.operands.size; i++) {
        accum = accum != eval_expr(expr->nary.operands.exprs[i], ctx);
      }
      return accum;
    }
    case ARVM_OP_TH2: {
      bool has_true = false;
      for (size_t i = 0; i < expr->nary.operands.size; i++) {
        if (i == expr->nary.operands.size - 1 && !has_true)
          return ARVM_FALSE;
        if (eval_expr(expr->nary.operands.exprs[i], ctx)) {
          if (has_true)
            return ARVM_TRUE;
          else
            has_true = true;
        }
      }
      return ARVM_FALSE;
    }
    default:
      unreachable();
    }
    break;
  case RANGE:
    return ctx.arg >= expr->range.min && ctx.arg <= expr->range.max;
  case MODEQ:
    return ctx.arg % expr->modeq.divisor == expr->modeq.residue;
  case CALL:
    return ctx.arg > expr->call.offset
               ? arvm_eval(expr->call.func, ctx.arg - expr->call.offset)
               : ARVM_FALSE;
  default:
    unreachable();
  }
}

arvm_val_t arvm_eval(arvm_func_t func, arvm_val_t arg) {
  return eval_expr(func->value, (eval_ctx_t){arg});
}