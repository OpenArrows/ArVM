#include "eval.h"

arvm_val_t eval_binary(arvm_binary_expr_t *expr, arvm_ctx_t ctx) {
  switch (expr->op) {
  case OR:
    return eval_expr(expr->lhs, ctx) || eval_expr(expr->rhs, ctx);
  case AND:
    return eval_expr(expr->lhs, ctx) && eval_expr(expr->rhs, ctx);
  case XOR:
    return eval_expr(expr->lhs, ctx) != eval_expr(expr->rhs, ctx);
  case ADD:
    return eval_expr(expr->lhs, ctx) + eval_expr(expr->rhs, ctx);
  case MOD:
    return eval_expr(expr->lhs, ctx) % eval_expr(expr->rhs, ctx);
  }
}

arvm_val_t eval_in_interval(arvm_in_interval_expr_t *expr, arvm_ctx_t ctx) {
  arvm_val_t value = eval_expr(expr->value, ctx);
  return value >= expr->min && value <= expr->max;
}

arvm_val_t eval_ref(arvm_ref_expr_t *expr, arvm_ctx_t ctx) {
  switch (expr->ref) {
  case ARG:
    return ctx.arg;
  }
}

arvm_val_t eval_call(arvm_call_expr_t *expr, arvm_ctx_t ctx) {
  return eval(expr->target, eval_expr(expr->arg, ctx));
}

arvm_val_t eval_const(arvm_const_expr_t *expr) { return expr->value; }

arvm_val_t eval_expr(arvm_expr_t *expr, arvm_ctx_t ctx) {
  switch (expr->kind) {
  case BINARY:
    return eval_binary(&expr->binary, ctx);
  case IN_INTERVAL:
    return eval_in_interval(&expr->in_interval, ctx);
  case REF:
    return eval_ref(&expr->ref, ctx);
  case CALL:
    return eval_call(&expr->call, ctx);
  case CONST:
    return eval_const(&expr->const_);
  case NONE:
    return 0;
  }
}

arvm_val_t eval(arvm_func_t *func, arvm_val_t arg) {
  if (arg <= 0)
    return ARVM_FALSE;
  return eval_expr(func->value, (arvm_ctx_t){arg});
}