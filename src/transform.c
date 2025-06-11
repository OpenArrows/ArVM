#include "transform.h"
#include "builder.h"
#include "visit.h"
#include <string.h>

#define TRANSFORM_ARENA_BLOCK_SIZE 16

typedef struct replace_ctx {
  arena_t *arena;
  pattern_t *what;
  arvm_expr_t *with;
} replace_ctx_t;

void transpose(arena_t *arena, const arvm_expr_t *what, arvm_expr_t *where) {
  arena_t temp_arena = {sizeof(arvm_expr_t) * TRANSFORM_ARENA_BLOCK_SIZE};
  arvm_expr_t *what_copy = make_clone(&temp_arena, what);
  copy_expr(arena, what_copy, where);
  arena_free(&temp_arena);
}

void nary_remove(arvm_expr_t *nary, arvm_expr_t *arg) {
  for (int i = 0; i < nary->nary.args.size; i++) {
    if (nary->nary.args.exprs[i] == arg) {
      nary->nary.args.size--;
      memcpy(&nary->nary.args.exprs[i], &nary->nary.args.exprs[i + 1],
             sizeof(arvm_expr_t *) * (nary->nary.args.size - i));
      break;
    }
  }
}

static void replace_visitor(arvm_expr_t *expr, void *ctx_) {
  replace_ctx_t *ctx = ctx_;
  if (matches(expr, ctx->what))
    transpose(ctx->arena, ctx->with, expr);
}

void replace(arena_t *arena, arvm_expr_t *where, pattern_t *what,
             arvm_expr_t *with) {
  visit(where, replace_visitor, &(replace_ctx_t){arena, what, with});
}