#include "eval.h"
#include <stddef.h>

arvm_val_t eval_nary(arvm_nary_expr_t *expr, arvm_ctx_t ctx) {
  switch (expr->op) {
  case OR: {
    for (int i = 0; i < expr->args.size; i++) {
      if (eval_expr(expr->args.exprs[i], ctx))
        return ARVM_TRUE;
    }
    return ARVM_FALSE;
  }
  case AND: {
    for (int i = 0; i < expr->args.size; i++) {
      if (!eval_expr(expr->args.exprs[i], ctx))
        return ARVM_FALSE;
    }
    return ARVM_TRUE;
  }
  case XOR: {
    arvm_val_t accum = ARVM_FALSE;
    for (int i = 0; i < expr->args.size; i++) {
      accum = accum != eval_expr(expr->args.exprs[i], ctx);
    }
    return accum;
  }
  case ADD: {
    arvm_val_t accum = 0;
    for (int i = 0; i < expr->args.size; i++) {
      accum += eval_expr(expr->args.exprs[i], ctx);
    }
    return accum;
  }
  }
}

arvm_val_t eval_in_interval(arvm_in_interval_expr_t *expr, arvm_ctx_t ctx) {
  arvm_val_t value = eval_expr(expr->value, ctx);
  return value >= expr->min && value <= expr->max;
}

arvm_val_t eval_call(arvm_call_expr_t *expr, arvm_ctx_t ctx) {
  return eval(expr->target, eval_expr(expr->arg, ctx));
}

arvm_val_t eval_const(arvm_const_expr_t *expr) { return expr->value; }

arvm_val_t eval_expr(arvm_expr_t *expr, arvm_ctx_t ctx) {
  switch (expr->kind) {
  case NARY:
    return eval_nary(&expr->nary, ctx);
  case IN_INTERVAL:
    return eval_in_interval(&expr->in_interval, ctx);
  case ARG_REF:
    return ctx.arg;
  case CALL:
    return eval_call(&expr->call, ctx);
  case CONST:
    return eval_const(&expr->const_);
  default:
    unreachable();
  }
}

arvm_val_t eval(arvm_func_t *func, arvm_val_t arg) {
  if (arg <= 0)
    return ARVM_FALSE;
  return eval_expr(func->value, (arvm_ctx_t){arg});
}