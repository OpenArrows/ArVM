#include "opt.h"
#include "analyze.h"
#include "builder.h"
#include "eval.h"
#include "imath.h"
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
    copy_expr(ctx->tmp_arena, expr, prev);

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
        arvm_expr_t *a, *b;
        while (matches(expr, NARY(ANYVAL(), CONST_AS(a, ANYVAL()),
                                  CONST_AS(b, ANYVAL())))) {
          arvm_val_t value = eval_nary(
              &(arvm_nary_expr_t){.op = expr->nary.op,
                                  .args = {2, (arvm_expr_t *[]){a, b}}},
              (arvm_ctx_t){0});
          arvm_expr_t const_ = {CONST, .const_ = {value}};
          transpose(ctx->arena, &const_, a);
          nary_remove(expr, b);
        }
      }
    } // General n-ary optimizations

    { // Interval optimizations
      {
        // Interval normalization (move const to RHS)
        arvm_expr_t *nary, *const_;
        if (matches(expr, IN_INTERVAL(NARY_AS(nary, VAL(ADD),
                                              CONST_AS(const_, ANYVAL())),
                                      ANYVAL(), ANYVAL()))) {
          nary_remove(nary, const_);
          visit(nary, arvm_optimize, ctx);
          expr->in_interval.min =
              iadd(expr->in_interval.min, -const_->const_.value);
          expr->in_interval.max =
              iadd(expr->in_interval.max, -const_->const_.value);
        }
      }

      {
        // Compile-time interval evaluation
        if (matches(expr, IN_INTERVAL(CONST(ANYVAL()), ANYVAL(), ANYVAL()))) {
          transpose(
              ctx->arena,
              make_const(ctx->tmp_arena,
                         eval_in_interval(&expr->in_interval, (arvm_ctx_t){0})),
              expr);
          return;
        }
      }

      {
        // Merge intervals in logical expressions
        arvm_expr_t *a, *b;
        FOR_EACH_MATCH(expr,
                       NARY(VAL(OR, AND),
                            IN_INTERVAL_AS(a, SLOT(), ANYVAL(), ANYVAL()),
                            IN_INTERVAL_AS(b, SLOT(), ANYVAL(), ANYVAL())),
                       {
                         if (intervals_overlap(a, b)) {
                           nary_remove(expr, b);
                           switch (expr->nary.op) {
                           case OR:
                             a->in_interval.min =
                                 imin(a->in_interval.min, b->in_interval.min);
                             a->in_interval.max =
                                 imax(a->in_interval.max, b->in_interval.max);
                             break;
                           case AND:
                             a->in_interval.min =
                                 imax(a->in_interval.min, b->in_interval.min);
                             a->in_interval.max =
                                 imin(a->in_interval.max, b->in_interval.max);
                             break;
                           default:
                             unreachable();
                           }
                           break;
                         }
                       });
      }
    } // Interval optimizations

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
        while (matches(expr, NARY(VAL(OR, AND), SLOT_AS(arg), SLOT()))) {
          nary_remove(expr, arg);
        }
      }

      {
        // Absorption law
        arvm_expr_t *nary;
        while (matches(expr, NARY(VAL(OR), NARY_AS(nary, VAL(AND), SLOT()),
                                  SLOT())) ||
               matches(expr, NARY(VAL(AND), NARY_AS(nary, VAL(OR), SLOT()),
                                  SLOT()))) {
          nary_remove(expr, nary);
        }
      }

      {
        // Distributive law
        arvm_expr_t *nary;
        bool matched = false;
        while (matches(expr, NARY(VAL(AND), NARY_AS(nary, VAL(OR))))) {
          matched = true;
          nary_remove(expr, nary);
          for (int i = 0; i < nary->nary.args.size; i++) {
            arvm_expr_t *arg = nary->nary.args.exprs[i];

            arvm_expr_t *new_arg = nary->nary.args.exprs[i] =
                make_expr(ctx->arena, NARY);
            new_arg->nary.op = AND;
            new_arg->nary.args.size = expr->nary.args.size + 1;
            new_arg->nary.args.exprs = arena_alloc(
                ctx->arena, sizeof(arvm_expr_t *) * new_arg->nary.args.size);
            for (int i = 0; i < expr->nary.args.size; i++)
              new_arg->nary.args.exprs[i] =
                  make_clone(ctx->arena, expr->nary.args.exprs[i]);
            new_arg->nary.args.exprs[expr->nary.args.size] = arg;
          }
          transpose(ctx->arena, nary, expr);
        }
        if (matched) {
          visit(expr, arvm_optimize, ctx); // TODO: remove recursion if possible
          return;
        }
      }
    } // Boolean laws

    { // Call optimizations
      {
        // Call inlining
        if (matches(expr, CALL(ANYVAL(), ANY()))) {
          arvm_func_t *func = expr->call.target;
          if (func == NULL)
            goto noinline;

          // We don't want to get stuck when inlining recursive functions
          arvm_opt_ctx_t *context = ctx;
          while (context) {
            if (func == context->func)
              goto noinline;
            context = context->parent;
          }

          // Besides just inlining the function, we also need to check that the
          // argument value is > 0, because functions are only defined for
          // positive arguments
          arvm_expr_t *func_val = make_clone(ctx->tmp_arena, func->value);
          replace(ctx->tmp_arena, func_val, ARG_REF(), expr->call.arg);
          transpose(ctx->arena,
                    make_nary(ctx->tmp_arena, AND, 2, func_val,
                              make_in_interval(
                                  ctx->tmp_arena,
                                  make_clone(ctx->tmp_arena, expr->call.arg), 1,
                                  ARVM_POSITIVE_INFINITY)),
                    expr);

          visit(expr, arvm_optimize,
                &(arvm_opt_ctx_t){ctx, ctx->tmp_arena, ctx->arena,
                                  func}); // TODO: do we need recursion here?
          return;
        }
      noinline:;
      }

      {
        // Constant-step recursion optimization
        arvm_expr_t *base, *call_nary, *step, *condition;
        if (matches(
                expr,
                NARY_FIXED(
                    VAL(OR),
                    IN_INTERVAL_AS(base, ARG_REF(), SLOTVAL(), SLOTVAL()),
                    NARY_FIXED_AS(
                        call_nary, VAL(AND),
                        CALL(VAL((arvm_val_t)ctx->func),
                             NARY_FIXED(
                                 VAL(ADD), ARG_REF(),
                                 CONST_AS(step, RANGEVAL(ARVM_NEGATIVE_INFINITY,
                                                         -1)))),
                        IN_INTERVAL_AS(condition, ARG_REF(), SLOTVAL(),
                                       VAL(ARVM_POSITIVE_INFINITY)))))) {
          expr->nary.op = AND;
          base->in_interval.value =
              make_binary(ctx->arena, MOD, base->in_interval.value,
                          make_const(ctx->arena, -step->const_.value));
          base->in_interval.min = base->in_interval.max =
              base->in_interval.min % -step->const_.value;
          transpose(ctx->arena, condition, call_nary);
        }
      }
    } // Call optimizations

  } while (!is_identical(
      prev, expr)); // Repeat optimizations until no transformation is applied

  if (expr->kind == NARY) {
    // Sort n-ary operands to allow short-circuit evaluation
    int i = 0, end = expr->nary.args.size - 1;
    while (i <= end) {
      arvm_expr_t *arg = expr->nary.args.exprs[i];
      if (has_calls(arg)) {
        expr->nary.args.exprs[i] = expr->nary.args.exprs[end];
        expr->nary.args.exprs[end] = arg;
        end--;
      } else {
        i++;
      }
    }
  }
}

void arvm_optimize_fn(arvm_func_t *func, arena_t *arena) {
  arena_t tmp_arena = (arena_t){sizeof(arvm_expr_t) * OPT_ARENA_BLOCK_SIZE};
  visit(func->value, arvm_optimize,
        &(arvm_opt_ctx_t){NULL, &tmp_arena, arena, func});
  arena_free(&tmp_arena);
}