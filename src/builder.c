#include "builder.h"
#include <stdlib.h>
#include <string.h>

arvm_expr_t *make_binary(arena_t *arena, arvm_binop_t op, arvm_expr_t *lhs,
                         arvm_expr_t *rhs) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = BINARY;
  expr->binary.op = op;
  expr->binary.lhs = lhs;
  expr->binary.rhs = rhs;
  return expr;
}

arvm_expr_t *make_in_interval(arena_t *arena, arvm_expr_t *value,
                              arvm_val_t min, arvm_val_t max) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = IN_INTERVAL;
  expr->in_interval.value = value;
  expr->in_interval.min = min;
  expr->in_interval.max = max;
  return expr;
}

arvm_expr_t *make_ref(arena_t *arena, arvm_ref_t ref) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = REF;
  expr->ref.ref = ref;
  return expr;
}

arvm_expr_t *make_call(arena_t *arena, arvm_func_t *target, arvm_expr_t *arg) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = CALL;
  expr->call.target = target;
  expr->call.arg = arg;
  return expr;
}

arvm_expr_t *make_const(arena_t *arena, arvm_val_t value) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = CONST;
  expr->const_.value = value;
  return expr;
}

static arvm_expr_t *create_or_reuse_expr(arena_t *arena, arvm_expr_t *old) {
  if (old != NULL) {
    old->kind = NONE;
    return old;
  }
  return arena_alloc(arena, sizeof(arvm_expr_t));
}

void clone_expr(arena_t *arena, const arvm_expr_t *src, arvm_expr_t *dst) {
  arvm_expr_t expr;
  memcpy(&expr, src, sizeof(expr));

  arvm_expr_t *subexprs[2] = {NULL};
  switch (dst->kind) {
  case BINARY:
    subexprs[0] = dst->binary.lhs;
    subexprs[1] = dst->binary.rhs;
    break;
  case IN_INTERVAL:
    subexprs[0] = dst->in_interval.value;
    break;
  case CALL:
    subexprs[0] = dst->call.arg;
    break;
  default:
    break;
  }

  dst->kind = expr.kind;
  switch (expr.kind) {
  case BINARY:
    dst->binary.op = expr.binary.op;
    dst->binary.lhs = create_or_reuse_expr(arena, subexprs[0]);
    clone_expr(arena, expr.binary.lhs, dst->binary.lhs);
    dst->binary.rhs = create_or_reuse_expr(arena, subexprs[1]);
    clone_expr(arena, expr.binary.rhs, dst->binary.rhs);
    break;
  case IN_INTERVAL:
    dst->in_interval.value = create_or_reuse_expr(arena, subexprs[0]);
    clone_expr(arena, expr.in_interval.value, dst->in_interval.value);
    dst->in_interval.min = expr.in_interval.min;
    dst->in_interval.max = expr.in_interval.max;
    break;
  case REF:
    dst->ref.ref = expr.ref.ref;
    break;
  case CALL:
    dst->call.target = expr.call.target;
    dst->call.arg = create_or_reuse_expr(arena, subexprs[0]);
    clone_expr(arena, expr.call.arg, dst->call.arg);
    break;
  case CONST:
    dst->const_.value = expr.const_.value;
    break;
  case NONE:
    break;
  }
}