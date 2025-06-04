#include "arena.h"
#include "arvm.h"

typedef struct opt_context arvm_opt_ctx_t;

struct opt_context {
  arvm_opt_ctx_t *parent;
  arena_t *tmp_arena;
  arena_t *arena;
  arvm_func_t *func;
  arvm_expr_t *arg;
};

void arvm_optimize(arvm_expr_t *expr, void *ctx);

void arvm_optimize_fn(arvm_func_t *func, arena_t *arena);