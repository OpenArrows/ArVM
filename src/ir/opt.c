#include "opt.h"
#include "analyze.h"
#include "builder.h"
#include "eval.h"
#include "transform.h"
#include "visit.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define OPT_ARENA_BLOCK_SIZE 16

typedef struct opt_ctx opt_ctx_t;

struct opt_ctx {
  opt_ctx_t *parent;
  arvm_arena_t *arena;
  arvm_arena_t *tmp_arena;
  arvm_func_t func;
};

static void optimize_visitor(arvm_expr_t expr, void *ctx_) {
  opt_ctx_t *ctx = ctx_;

  /*
  arvm_expr_t prev;
  do {
    prev = arvm_clone(ctx->tmp_arena, expr);

    { // General n-ary optimizations
      {
        // Unwrap n-ary expressions
        arvm_expr_t child;
        if (matches(expr, NARY_FIXED(ANYVAL(), ANY_AS(child)))) {
          arvm_transpose(ctx->arena, child, expr);
          return;
        }
      }

      {
        // Fold n-ary expressions
        if (matches(expr, NARY(ANYVAL(), NARY(VAL(expr->nary.op))))) {
          size_t operand_count = expr->nary.operands.size;
          arvm_expr_t *operands = expr->nary.operands.exprs;

          expr->nary.operands.size = 0;
          for (int i = 0; i < operand_count; i++) {
            arvm_expr_t operand = operands[i];
            expr->nary.operands.size +=
                operand->kind == NARY && operand->nary.op == expr->nary.op
                    ? operand->nary.operands.size
                    : 1;
          }

          expr->nary.operands.exprs = arvm_arena_alloc(
              ctx->arena, sizeof(arvm_expr_t) * expr->nary.operands.size);

          size_t offset = 0;
          for (int i = 0; i < operand_count; i++) {
            arvm_expr_t operand = operands[i];
            if (operand->kind == NARY && operand->nary.op == expr->nary.op) {
              memcpy(&expr->nary.operands.exprs[offset],
                     operand->nary.operands.exprs,
                     sizeof(arvm_expr_t) * operand->nary.operands.size);
              offset += operand->nary.operands.size;
            } else {
              expr->nary.operands.exprs[offset] = operand;
              offset++;
            }
          }
        }
      }

      {
        // Compile-time n-ary evaluation
        arvm_expr_t a, b;
        while (matches(expr, NARY(ANYVAL(), CONST_AS(a, ANYVAL()),
                                  CONST_AS(b, ANYVAL())))) {
          arvm_val_t value = arvm_eval_expr(&(struct arvm_expr){
              NARY, .nary = {.op = expr->nary.op,
                             .operands = {2, (arvm_expr_t[]){a, b}}}});
          struct arvm_expr const_ = {CONST, .const_ = {value}};
          arvm_transpose(ctx->arena, &const_, a);
          arvm_nary_remove_operand(expr, b);
        }
      }
    } // General n-ary optimizations

    { // Interval optimizations
      {
        // Interval normalization (move const to RHS)
        arvm_expr_t nary, const_;
        if (matches(expr, IN_INTERVAL(NARY_AS(nary, VAL(ARVM_NARY_TH2),
                                              CONST_AS(const_, ANYVAL())),
                                      ANYVAL(), ANYVAL()))) {
          arvm_nary_remove_operand(nary, const_);
          arvm_visit(nary, optimize_visitor, ctx);
          if (expr->range.min != ARVM_NEGATIVE_INFINITY)
            expr->range.min -= const_->const_.value;
          if (expr->range.max != ARVM_POSITIVE_INFINITY)
            expr->range.max -= const_->const_.value;
        }
      }

      {
        // Compile-time interval evaluation
        if (matches(expr, RANGE(CONST(ANYVAL()), ANYVAL(), ANYVAL()))) {
          arvm_val_t result = arvm_eval_expr(expr);
          struct arvm_expr const_ = {CONST, .const_ = {result}};
          arvm_transpose(ctx->arena, &const_, expr);
          return;
        }
      }

      {
        // Merge intervals in logical expressions
        arvm_expr_t a, b;
        FOR_EACH_MATCH(expr,
                       NARY(VAL(ARVM_NARY_OR, ARVM_NARY_AND),
                            RANGE_AS(a, SLOT(), ANYVAL(), ANYVAL()),
                            RANGE_AS(b, SLOT(), ANYVAL(), ANYVAL())),
                       {
                         if (arvm_ranges_overlap(a, b)) {
                           arvm_nary_remove_operand(expr, b);
                           switch (expr->nary.op) {
                           case ARVM_NARY_OR:
                             if (b->range.min < a->range.min)
                               a->range.min = b->range.min;
                             if (b->range.max > a->range.max)
                               a->range.max = b->range.max;
                             break;
                           case ARVM_NARY_AND:
                             if (b->range.min > a->range.min)
                               a->range.min = b->range.min;
                             if (b->range.max < a->range.max)
                               a->range.max = b->range.max;
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
        arvm_expr_t arg;
        if (matches(expr, NARY(VAL(ARVM_NARY_OR), CONST_AS(arg, VAL(1)))) ||
            matches(expr, NARY(VAL(ARVM_NARY_AND), CONST_AS(arg, VAL(0))))) {
          arvm_transpose(ctx->arena, arg, expr);
          return;
        }
      }

      {
        // Identity law
        arvm_expr_t arg;
        while (matches(expr, NARY(VAL(ARVM_NARY_OR), CONST_AS(arg, VAL(0)))) ||
               matches(expr, NARY(VAL(ARVM_NARY_AND), CONST_AS(arg, VAL(1))))) {
          arvm_nary_remove_operand(expr, arg);
        }
      }

      {
        // Idempotent law
        arvm_expr_t arg;
        while (matches(expr, NARY(VAL(ARVM_NARY_OR, ARVM_NARY_AND),
                                  SLOT_AS(arg), SLOT()))) {
          arvm_nary_remove_operand(expr, arg);
        }
      }

      {
        // Absorption law
        arvm_expr_t nary;
        while (matches(expr, NARY(VAL(ARVM_NARY_OR),
                                  NARY_AS(nary, VAL(ARVM_NARY_AND), SLOT()),
                                  SLOT())) ||
               matches(expr, NARY(VAL(ARVM_NARY_AND),
                                  NARY_AS(nary, VAL(ARVM_NARY_OR), SLOT()),
                                  SLOT()))) {
          arvm_nary_remove_operand(expr, nary);
        }
      }

      {
        // Distributive law
        arvm_expr_t nary;
        bool matched = false;
        while (matches(
            expr, NARY(VAL(ARVM_NARY_AND), NARY_AS(nary, VAL(ARVM_NARY_OR))))) {
          matched = true;
          arvm_nary_remove_operand(expr, nary);
          for (int i = 0; i < nary->nary.operands.size; i++) {
            arvm_expr_t arg = nary->nary.operands.exprs[i];

            arvm_expr_t new_arg = nary->nary.operands.exprs[i] =
                arvm_new_expr(ctx->arena, NARY);
            new_arg->nary.op = ARVM_NARY_AND;
            new_arg->nary.operands.size = expr->nary.operands.size + 1;
            new_arg->nary.operands.exprs = arvm_arena_alloc(
                ctx->arena, sizeof(arvm_expr_t) * new_arg->nary.operands.size);
            for (int i = 0; i < expr->nary.operands.size; i++)
              new_arg->nary.operands.exprs[i] =
                  arvm_clone(ctx->arena, expr->nary.operands.exprs[i]);
            new_arg->nary.operands.exprs[expr->nary.operands.size] = arg;
          }
          arvm_transpose(ctx->arena, nary, expr);
        }
        if (matched) {
          arvm_visit(expr, optimize_visitor,
                     ctx); // TODO: remove recursion if possible
          return;
        }
      }
    } // Boolean laws

    { // Call optimizations
      {
        // Call inlining
        if (matches(expr, CALL(ANYVAL(), ANY()))) {
          arvm_func_t func = expr->call.func;
          if (func == NULL)
            goto noinline;

          // We don't want to get stuck when inlining recursive functions
          opt_ctx_t *context = ctx;
          while (context) {
            if (func == context->func)
              goto noinline;
            context = context->parent;
          }

          // Besides just inlining the function, we also need to check that the
          // argument value is > 0, because functions are only defined for
          // positive arguments
          // TODO: replace arena allocations with local variable pointers
          arvm_expr_t func_val = arvm_clone(ctx->tmp_arena, func->value);
          arvm_replace(ctx->tmp_arena, func_val, ARG_REF(), expr->call.arg);
          arvm_transpose(
              ctx->arena,
              arvm_new_nary(
                  ctx->tmp_arena, ARVM_NARY_AND, 2, func_val,
                  arvm_new_range(ctx->tmp_arena,
                                 arvm_clone(ctx->tmp_arena, expr->call.arg), 1,
                                 ARVM_POSITIVE_INFINITY)),
              expr);

          arvm_visit(expr, optimize_visitor,
                     &(opt_ctx_t){ctx, ctx->arena, ctx->tmp_arena,
                                  func}); // TODO: do we need recursion here?
          return;
        }
      noinline:;
      }

      {
        // Constant-step recursion optimization
        arvm_expr_t base, call_nary, step, condition;
        if (matches(
                expr,
                NARY_FIXED(
                    VAL(ARVM_NARY_OR),
                    RANGE_AS(base, ARG_REF(), SLOTVAL(), SLOTVAL()),
                    NARY_FIXED_AS(
                        call_nary, VAL(ARVM_NARY_AND),
                        CALL(VAL((arvm_val_t)ctx->func),
                             NARY_FIXED(
                                 VAL(ARVM_NARY_TH2), ARG_REF(),
                                 CONST_AS(step, RANGEVAL(ARVM_NEGATIVE_INFINITY,
                                                         -1)))),
                        RANGE_AS(condition, ARG_REF(), SLOTVAL(),
                                 VAL(ARVM_POSITIVE_INFINITY)))))) {
          expr->nary.op = ARVM_NARY_AND;
          base->range.value =
              arvm_new_binary(ctx->arena, ARVM_BINARY_MOD, base->range.value,
                              arvm_new_const(ctx->arena, -step->const_.value));
          base->range.min = base->range.max =
              base->range.min % -step->const_.value;
          arvm_transpose(ctx->arena, condition, call_nary);
        }
      }
    } // Call optimizations

  } while (!arvm_is_identical(
      prev, expr)); // Repeat optimizations until no transformation is applied
  */

  if (expr->kind == NARY) {
    // Sort n-ary operands to allow short-circuit evaluation
    int i = 0, end = expr->nary.operands.size - 1;
    while (i <= end) {
      arvm_expr_t arg = expr->nary.operands.exprs[i];
      if (arvm_has_calls(arg)) {
        expr->nary.operands.exprs[i] = expr->nary.operands.exprs[end];
        expr->nary.operands.exprs[end] = arg;
        end--;
      } else {
        i++;
      }
    }
  }
}

void arvm_optimize_func(arvm_func_t func, arvm_arena_t *arena,
                        arvm_arena_t *temp_arena) {
  arvm_visit(func->value, optimize_visitor,
             &(opt_ctx_t){NULL, temp_arena, arena, func});
}