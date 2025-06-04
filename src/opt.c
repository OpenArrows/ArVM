#include "opt.h"
#include "analyze.h"
#include "eval.h"
#include "imath.h"
#include "match.h"
#include "transform.h"
#include "visit.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define OPT_ARENA_BLOCK_SIZE 16

void arvm_optimize(arvm_expr_t *expr, void *ctx_) {
  arvm_opt_ctx_t *ctx = ctx_;

  arvm_expr_t *prev = make_expr(ctx->tmp_arena, NONE);
  do {
    clone_expr(ctx->tmp_arena, expr, prev);

    { // General n-ary optimizations
      {
        // Unwrap n-ary expressions
        arvm_expr_t *child;
        if (matches(expr, NARY_FIXED(ANYVAL(), ANY_AS(child)))) {
          transpose(ctx->arena, child, expr);
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

          expr->nary.args.exprs = arena_alloc(
              ctx->arena, sizeof(arvm_expr_t *) * expr->nary.args.size);

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
          transpose(ctx->arena, &const_, expr);
          return;
        }
      }
    } // General n-ary optimizations

    { // Boolean laws
      {
        // Annulment law
        arvm_expr_t *arg;
        if (matches(expr, NARY(VAL(OR), CONST_AS(arg, VAL(1)))) ||
            matches(expr, NARY(VAL(AND), CONST_AS(arg, VAL(0))))) {
          transpose(ctx->arena, arg, expr);
          return;
        }
      }

      {
        // Identity law
        arvm_expr_t *arg;
        while (matches(expr, NARY(VAL(OR), CONST_AS(arg, VAL(0)))) ||
               matches(expr, NARY(VAL(AND), CONST_AS(arg, VAL(1))))) {
          nary_remove(expr, arg);
        }
      }

      {
        // Idempotent law
        arvm_expr_t *arg;
        while (matches(expr, NARY(VAL(OR), SLOT_AS(arg), SLOT())) ||
               matches(expr, NARY(VAL(AND), SLOT_AS(arg), SLOT()))) {
          nary_remove(expr, arg);
        }
      }
    } // Boolean laws

  } while (!is_identical(
      prev, expr)); // Repeat optimizations until no transformation is applied
}

void arvm_optimize_fn(arvm_func_t *func, arena_t *arena) {
  arena_t tmp_arena = (arena_t){sizeof(arvm_expr_t) * OPT_ARENA_BLOCK_SIZE};
  visit(func->value, arvm_optimize,
        &(arvm_opt_ctx_t){NULL, &tmp_arena, arena, func, NULL});
  arena_free(&tmp_arena);
}