#include "transform.h"
#include "builder.h"
#include "visit.h"
#include <string.h>

#define TEMP_ARENA_BLOCK_SIZE sizeof(struct arvm_expr) * 8

typedef struct replace_ctx {
  arvm_arena_t *arena;
  pattern_t *what;
  arvm_expr_t with;
} replace_ctx_t;

void arvm_transpose(arvm_arena_t *arena, const arvm_expr_t what,
                    arvm_expr_t where) {
  arvm_arena_t temp_arena = {TEMP_ARENA_BLOCK_SIZE};
  arvm_expr_t what_copy = arvm_clone(&temp_arena, what);
  arvm_copy_expr(arena, what_copy, where);
  arvm_arena_free(&temp_arena);
}

void arvm_nary_remove_operand(arvm_expr_t nary, arvm_expr_t operand) {
  for (int i = 0; i < nary->nary.operands.size; i++) {
    if (nary->nary.operands.exprs[i] == operand) {
      nary->nary.operands.size--;
      memcpy(&nary->nary.operands.exprs[i], &nary->nary.operands.exprs[i + 1],
             sizeof(arvm_expr_t) * (nary->nary.operands.size - i));
      break;
    }
  }
}

static void replace_visitor(arvm_expr_t expr, void *ctx_) {
  replace_ctx_t *ctx = ctx_;
  if (matches(expr, ctx->what))
    arvm_transpose(ctx->arena, ctx->with, expr);
}

void arvm_replace(arvm_arena_t *arena, arvm_expr_t where, pattern_t *what,
                  arvm_expr_t with) {
  arvm_visit(where, replace_visitor, &(replace_ctx_t){arena, what, with});
}