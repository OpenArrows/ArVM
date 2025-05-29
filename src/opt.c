#include "opt.h"
#include "analyze.h"
#include "builder.h"
#include "eval.h"
#include "imath.h"
#include "visit.h"
#include <stdbool.h>
#include <stdlib.h>

static void binary_swap(arvm_binary_expr_t *expr) {
  if (!(expr->op == OR || expr->op == AND || expr->op == XOR ||
        expr->op == ADD))
    return;

  arvm_expr_t *rhs = expr->rhs;
  expr->rhs = expr->lhs;
  expr->lhs = rhs;
}

// TODO: is this needed
static void substitute(arvm_expr_t *expr, arvm_expr_t *with,
                       arvm_opt_ctx_t *ctx) {
  clone_expr(ctx->arena, with, expr);
  visit(expr, arvm_optimize, ctx);
}

void arvm_optimize(arvm_expr_t *expr, void *ctx_) {
  arvm_opt_ctx_t *ctx = ctx_;

  switch (expr->kind) {
  case BINARY: {
    if (expr->binary.lhs->kind == CONST) {
      if (expr->binary.rhs->kind == CONST) {
        // If we encounter constant expressions on both sides, evaluate the
        // binary at compile time
        arvm_val_t value = eval_binary(&expr->binary, (arvm_ctx_t){0});
        expr->kind = CONST;
        expr->const_.value = value;
        break;
      } else {
        // If only the LHS is constant, we move it to the RHS to simplify
        // further pattern matching
        binary_swap(&expr->binary);
      }
    }

    // If the LHS is an 'in interval' expression, we also move it to the RHS
    // (unless RHS is constant)
    if (expr->binary.lhs->kind == IN_INTERVAL &&
        expr->binary.rhs->kind != CONST)
      binary_swap(&expr->binary);

    arvm_expr_t *lhs = expr->binary.lhs;
    arvm_expr_t *rhs = expr->binary.rhs;

    bool rhsConst = rhs->kind == CONST;

    switch (expr->binary.op) {
    case OR:
      if (lhs->kind == BINARY) {
        if (lhs->binary.op == AND) {
          if (is_identical(lhs->binary.rhs, rhs)) {
            clone_expr(ctx->arena, rhs, expr);
          }
        }
      }
      if (rhsConst) {
        if (rhs->const_.value)
          clone_expr(ctx->arena, rhs, expr); // x | true => true
        else
          clone_expr(ctx->arena, lhs, expr); // x | false => x
      } else if (rhs->kind == IN_INTERVAL) {
        arvm_expr_t *lhsInterval;
        if (lhs->kind == IN_INTERVAL)
          lhsInterval = lhs;
        else if (lhs->kind == BINARY && lhs->binary.op == OR &&
                 lhs->binary.rhs->kind == IN_INTERVAL) {
          lhsInterval = lhs->binary.rhs;
        } else
          break;
        if (!is_identical(lhsInterval->in_interval.value,
                          rhs->in_interval.value))
          return;
        // x in X | x in Y => x in (X JOIN Y)
        if (imin(lhsInterval->in_interval.max, rhs->in_interval.max) <
            imax(lhsInterval->in_interval.min, rhs->in_interval.min))
          break;
        lhsInterval->in_interval.min =
            imin(lhsInterval->in_interval.min, rhs->in_interval.min);
        lhsInterval->in_interval.max =
            imax(lhsInterval->in_interval.max, rhs->in_interval.max);
        clone_expr(ctx->arena, lhs, expr);
      }
      break;
    case AND:
      if (rhsConst) {
        if (rhs->const_.value)
          clone_expr(ctx->arena, lhs, expr); // x & true => x
        else
          clone_expr(ctx->arena, rhs, expr); // x & false => false
      } else if (rhs->kind == IN_INTERVAL) {
        arvm_expr_t *lhsInterval;
        if (lhs->kind == IN_INTERVAL)
          lhsInterval = lhs;
        else if (lhs->kind == BINARY && lhs->binary.op == AND &&
                 lhs->binary.rhs->kind == IN_INTERVAL) {
          lhsInterval = lhs->binary.rhs;
        } else
          break;
        if (!is_identical(lhsInterval->in_interval.value,
                          rhs->in_interval.value))
          return;
        // x in X & x in Y => x in (X INTERSECT Y)
        if (imin(lhsInterval->in_interval.max, rhs->in_interval.max) <
            imax(lhsInterval->in_interval.min, rhs->in_interval.min))
          break;
        lhsInterval->in_interval.min =
            imax(lhsInterval->in_interval.min, rhs->in_interval.min);
        lhsInterval->in_interval.max =
            imin(lhsInterval->in_interval.max, rhs->in_interval.max);
        clone_expr(ctx->arena, lhs, expr);
      }
      break;
    case ADD:
      if (rhsConst && lhs->kind == BINARY && lhs->binary.op == ADD &&
          lhs->binary.rhs->kind == CONST) {
        // x + C1 + C2 => x1 + (C1 + C2)
        arvm_val_t rhsValue = rhs->const_.value;
        clone_expr(ctx->arena, lhs, expr);
        expr->binary.rhs->const_.value += rhsValue;
      }
      break;
    default:
      break;
    }
    break;
  }
  case IN_INTERVAL: {
    arvm_expr_t *value = expr->in_interval.value;
    arvm_expr_t *rhs = value->binary.rhs;
    if (value->kind == BINARY && value->binary.op == ADD &&
        rhs->kind == CONST) {
      // x + C in [A, B] => x in [A - C, B - C]
      expr->in_interval.min = iadd(-rhs->const_.value, expr->in_interval.min);
      expr->in_interval.max = iadd(-rhs->const_.value, expr->in_interval.max);
      clone_expr(ctx->arena, value->binary.lhs, value);
    }
    break;
  }
  case REF:
    // We need to manually substitute the argument references when we inline
    // functions, e.g.
    //   f1(t) = t + 4
    //   f2(t) = f1(t + -1) => (t + -1) + 4 (instead of t + 4, which is wrong)
    if (expr->ref.ref == ARG && ctx->arg != NULL)
      clone_expr(ctx->arena, ctx->arg, expr);
    break;
  case CALL: {
    // Inline function calls when possible
    arvm_opt_ctx_t *cur = ctx;
    while (cur != NULL) {
      if (expr->call.target == cur->func)
        return;
      cur = cur->parent;
    }
    arvm_func_t *func = expr->call.target;
    arvm_expr_t arg;
    clone_expr(ctx->arena, expr->call.arg, &arg);

    // Besides just inlining the function, we also need to check that the
    // argument value is > 0, because functions are only defined for positive
    // arguments
    arvm_expr_t arg_ref = (arvm_expr_t){
        REF, .ref = {ARG}}; // arg ref gets replaced with the actual argument
    arvm_expr_t arg_gt_zero = (arvm_expr_t){
        IN_INTERVAL, .in_interval = {&arg_ref, 1, ARVM_POSITIVE_INFINITY}};
    arvm_expr_t new_expr = {
        BINARY, .binary = {AND, expr->call.target->value, &arg_gt_zero}};
    clone_expr(ctx->arena, &new_expr, expr);

    visit(expr, arvm_optimize, &(arvm_opt_ctx_t){ctx, ctx->arena, func, &arg});
    break;
  }
  default:
    break;
  }
}

void arvm_optimize_fn(arvm_func_t *func, arena_t *arena) {
  visit(func->value, arvm_optimize, &(arvm_opt_ctx_t){NULL, arena, func, NULL});
}