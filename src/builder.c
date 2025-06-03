#include "builder.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

arvm_expr_t *make_expr(arena_t *arena, arvm_expr_kind_t kind) {
  arvm_expr_t *expr = arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = kind;
  return expr;
}

arvm_expr_t *make_nary(arena_t *arena, arvm_nary_op_t op, size_t arg_count,
                       ...) {
  arvm_expr_t *expr = make_expr(arena, NARY);
  expr->nary.op = op;
  expr->nary.args.size = arg_count;
  expr->nary.args.exprs = arena_alloc(arena, sizeof(arvm_expr_t *) * arg_count);
  va_list args;
  va_start(args, arg_count);
  for (int i = 0; i < arg_count; i++)
    expr->nary.args.exprs[i] = va_arg(args, arvm_expr_t *);
  va_end(args);
  return expr;
}

arvm_expr_t *make_in_interval(arena_t *arena, arvm_expr_t *value,
                              arvm_val_t min, arvm_val_t max) {
  arvm_expr_t *expr = make_expr(arena, IN_INTERVAL);
  expr->in_interval.value = value;
  expr->in_interval.min = min;
  expr->in_interval.max = max;
  return expr;
}

arvm_expr_t *make_arg_ref(arena_t *arena) { return make_expr(arena, ARG_REF); }

arvm_expr_t *make_call(arena_t *arena, arvm_func_t *target, arvm_expr_t *arg) {
  arvm_expr_t *expr = make_expr(arena, CALL);
  expr->call.target = target;
  expr->call.arg = arg;
  return expr;
}

arvm_expr_t *make_const(arena_t *arena, arvm_val_t value) {
  arvm_expr_t *expr = make_expr(arena, CONST);
  expr->const_.value = value;
  return expr;
}

static arvm_expr_t *create_or_reuse_expr(arena_t *arena,
                                         arvm_expr_t **reusables,
                                         size_t resuable_count, size_t idx,
                                         const arvm_expr_t *src) {
  arvm_expr_t *expr = idx < resuable_count
                          ? reusables[idx]
                          : arena_alloc(arena, sizeof(arvm_expr_t));
  expr->kind = NONE; // TODO: reuse reusables' children?
  clone_expr(arena, src, expr);
  return expr;
}

void clone_expr(arena_t *arena, const arvm_expr_t *src, arvm_expr_t *dst) {
  arvm_expr_t expr;
  memcpy(&expr, src, sizeof(expr));

  size_t subexpr_count;
  switch (dst->kind) {
  case NARY:
    subexpr_count = dst->nary.args.size;
    break;
  case IN_INTERVAL:
  case CALL:
    subexpr_count = 1;
    break;
  default:
    subexpr_count = 0;
    break;
  }

  arvm_expr_t *subexprs[subexpr_count];
  switch (dst->kind) {
  case NARY:
    memcpy(subexprs, dst->nary.args.exprs, sizeof(subexprs));
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
  case NARY:
    dst->nary.op = expr.nary.op;
    dst->nary.args.size = expr.nary.args.size;
    dst->nary.args.exprs =
        arena_alloc(arena, sizeof(arvm_expr_t *) *
                               expr.nary.args.size); // TODO: reuse memory
    for (int i = 0; i < expr.nary.args.size; i++) {
      dst->nary.args.exprs[i] = create_or_reuse_expr(
          arena, subexprs, subexpr_count, i, expr.nary.args.exprs[i]);
    }
    break;
  case IN_INTERVAL:
    dst->in_interval.value = create_or_reuse_expr(
        arena, subexprs, subexpr_count, 0, expr.in_interval.value);
    dst->in_interval.min = expr.in_interval.min;
    dst->in_interval.max = expr.in_interval.max;
    break;
  case CALL:
    dst->call.target = expr.call.target;
    dst->call.arg =
        create_or_reuse_expr(arena, subexprs, subexpr_count, 0, expr.call.arg);
    break;
  case CONST:
    dst->const_.value = expr.const_.value;
    break;
  case ARG_REF:
  case NONE:
    break;
  }
}