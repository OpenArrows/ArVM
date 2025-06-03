#include "opt.h"
#include "analyze.h"
#include "builder.h"
#include "eval.h"
#include "imath.h"
#include "match.h"
#include "visit.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// TODO: is this needed
static void substitute(arvm_expr_t *expr, arvm_expr_t *with,
                       arvm_opt_ctx_t *ctx) {
  clone_expr(ctx->arena, with, expr);
  visit(expr, arvm_optimize, ctx);
}

void arvm_optimize(arvm_expr_t *expr, void *ctx_) {
  arvm_opt_ctx_t *ctx = ctx_;

  {
    // Unwrap n-ary expressions
    arvm_expr_t *child;
    if (matches(expr, NARY_FIXED(ANYVAL(), ANY_AS(child)))) {
      clone_expr(ctx->arena, child, expr);
      return;
    }
  }

  {
    // Fold n-ary expressions
    if (matches(expr, NARY(ANYVAL(), NARY(VAL(expr->nary.op))))) {
      size_t argc = expr->nary.args.size;
      arvm_expr_t **argv = expr->nary.args.exprs;

      expr->nary.args.size = 0;
      for (int i = 0; i < argc; i++) {
        arvm_expr_t *arg = argv[i];
        expr->nary.args.size +=
            arg->kind == NARY && arg->nary.op == expr->nary.op
                ? arg->nary.args.size
                : 1;
      }

      expr->nary.args.exprs =
          arena_alloc(ctx->arena, sizeof(arvm_expr_t *) * expr->nary.args.size);

      size_t offset = 0;
      for (int i = 0; i < argc; i++) {
        arvm_expr_t *arg = argv[i];
        if (arg->kind == NARY && arg->nary.op == expr->nary.op) {
          memcpy(&expr->nary.args.exprs[offset], arg->nary.args.exprs,
                 sizeof(arvm_expr_t *) * arg->nary.args.size);
          offset += arg->nary.args.size;
        } else {
          expr->nary.args.exprs[offset] = arg;
          offset++;
        }
      }
    }
  }

  {
    // Compile-time n-ary evaluation
    if (matches(expr, NARY_EACH(ANYVAL(), CONST(ANYVAL())))) {
      arvm_val_t value = eval_nary(&expr->nary, (arvm_ctx_t){0});
      arvm_expr_t const_ = {CONST, .const_ = {value}};
      clone_expr(ctx->arena, &const_, expr);
      return;
    }
  }
}

void arvm_optimize_fn(arvm_func_t *func, arena_t *arena) {
  visit(func->value, arvm_optimize, &(arvm_opt_ctx_t){NULL, arena, func, NULL});
}