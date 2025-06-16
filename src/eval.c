#include "eval.h"
#include "ir.h"
#include <stddef.h>

typedef struct eval_ctx {
  arvm_val_t arg;
} eval_ctx_t;

static arvm_val_t eval_expr(arvm_expr_t expr, eval_ctx_t ctx) {
  switch (expr->kind) {
  case BINARY:
    switch (expr->binary.op) {
    case ARVM_BINARY_MOD:
      return eval_expr(expr->binary.lhs, ctx) %
             eval_expr(expr->binary.rhs, ctx);
    default:
      unreachable();
    }
    break;
  case NARY:
    switch (expr->nary.op) {
    case ARVM_NARY_OR: {
      for (int i = 0; i < expr->nary.operands.size; i++) {
        if (eval_expr(expr->nary.operands.exprs[i], ctx))
          return ARVM_TRUE;
      }
      return ARVM_FALSE;
    }
    case ARVM_NARY_AND: {
      for (int i = 0; i < expr->nary.operands.size; i++) {
        if (!eval_expr(expr->nary.operands.exprs[i], ctx))
          return ARVM_FALSE;
      }
      return ARVM_TRUE;
    }
    case ARVM_NARY_XOR: {
      arvm_val_t accum = ARVM_FALSE;
      for (int i = 0; i < expr->nary.operands.size; i++) {
        accum = accum != eval_expr(expr->nary.operands.exprs[i], ctx);
      }
      return accum;
    }
    case ARVM_NARY_ADD: {
      arvm_val_t accum = 0;
      for (int i = 0; i < expr->nary.operands.size; i++) {
        accum += eval_expr(expr->nary.operands.exprs[i], ctx);
      }
      return accum;
    }
    default:
      unreachable();
    }
    break;
  case IN_INTERVAL: {
    arvm_val_t value = eval_expr(expr->in_interval.value, ctx);
    return value >= expr->in_interval.min && value <= expr->in_interval.max;
  }
  case ARG_REF:
    return ctx.arg;
  case CALL:
    return arvm_eval(expr->call.func, eval_expr(expr->call.arg, ctx));
  case CONST:
    return expr->const_.value;
  default:
    unreachable();
  }
}

arvm_val_t arvm_eval_expr(arvm_expr_t expr) {
  return eval_expr(expr, (eval_ctx_t){0});
}

arvm_val_t arvm_eval(arvm_func_t func, arvm_val_t arg) {
  if (arg <= 0)
    return ARVM_FALSE;
  return eval_expr(func->value, (eval_ctx_t){arg});
}