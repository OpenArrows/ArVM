#include "arvm.h"
#include "arena.h"
#include "eval.h"
#include "ir/builder.h"
#include "ir/ir.h"
#include "ir/opt.h"
#include <stdarg.h>
#include <stdlib.h>

#define ARVM_EXPR_ARENA_BLOCK_SIZE sizeof(struct arvm_expr) * 2048
#define ARVM_OPT_ARENA_BLOCK_SIZE sizeof(struct arvm_expr) * 1024
#define ARVM_FUNC_ARENA_BLOCK_SIZE sizeof(struct arvm_func) * 256

#define ARVM_TEMP_ARENA_BLOCK_SIZE sizeof(struct arvm_expr) * 2048

struct arvm_ctx {
  // Unoptimzed input IR
  arvm_arena_t expr_arena;
  // Optimized IR
  arvm_arena_t opt_arena;
  // Generated functions
  arvm_arena_t func_arena;
};

arvm_ctx_t arvm_create_context() {
  arvm_ctx_t ctx = malloc(sizeof(struct arvm_ctx));
  *ctx = (struct arvm_ctx){
      .expr_arena = (arvm_arena_t){ARVM_EXPR_ARENA_BLOCK_SIZE},
      .opt_arena = (arvm_arena_t){ARVM_OPT_ARENA_BLOCK_SIZE},
      .func_arena = (arvm_arena_t){ARVM_FUNC_ARENA_BLOCK_SIZE},
  };
  return ctx;
}

void arvm_release_context(arvm_ctx_t ctx) {
  arvm_arena_free(&ctx->expr_arena);
  arvm_arena_free(&ctx->opt_arena);
  arvm_arena_free(&ctx->func_arena);
  free(ctx);
}

arvm_func_t arvm_create_function(arvm_ctx_t ctx, arvm_expr_t value) {
  arvm_func_t func = arvm_arena_alloc(&ctx->func_arena, sizeof(struct arvm_func));
  *func = (struct arvm_func){value};
  return func;
}

void arvm_finalize(arvm_ctx_t ctx) {
  arvm_arena_t temp_arena = {ARVM_TEMP_ARENA_BLOCK_SIZE};
  // TODO: go with a more explicit approach for iterating over each function
  // (e.g. use a list instead of arena)
  for (arvm_arena_block_t *block = ctx->func_arena.head; block != NULL;
       block = block->next)
    for (size_t i = 0; i < block->allocated / sizeof(struct arvm_func); i++) {
      arvm_func_t func = &((struct arvm_func *)(block + 1))[i];
      arvm_optimize_func(func, &ctx->opt_arena, &temp_arena);
    }
  arvm_arena_free(&temp_arena);
}

arvm_val_t arvm_call_function(arvm_func_t func, arvm_val_t arg) {
  return arvm_eval(func, arg);
}

arvm_expr_t arvm_make_binary(arvm_ctx_t ctx, arvm_binary_op_t op,
                             arvm_expr_t lhs, arvm_expr_t rhs) {
  return arvm_new_binary(&ctx->expr_arena, op, lhs, rhs);
}

arvm_expr_t arvm_make_nary(arvm_ctx_t ctx, arvm_nary_op_t op,
                           size_t operand_count, ...) {
  va_list args;
  va_start(args, operand_count);
  arvm_expr_t nary = arvm_new_nary_v(&ctx->expr_arena, op, operand_count, args);
  va_end(args);
  return nary;
}

arvm_expr_t arvm_make_in_interval(arvm_ctx_t ctx, arvm_expr_t value,
                                  arvm_val_t min, arvm_val_t max) {
  return arvm_new_in_interval(&ctx->expr_arena, value, min, max);
}

arvm_expr_t arvm_make_arg_ref(arvm_ctx_t ctx) {
  return arvm_new_arg_ref(&ctx->expr_arena);
}

arvm_expr_t arvm_make_call(arvm_ctx_t ctx, arvm_func_t func, arvm_expr_t arg) {
  return arvm_new_call(&ctx->expr_arena, func, arg);
}

arvm_expr_t arvm_make_const(arvm_ctx_t ctx, arvm_val_t value) {
  return arvm_new_const(&ctx->expr_arena, value);
}