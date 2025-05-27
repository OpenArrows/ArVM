#include "arena.h"
#include "arvm.h"

arvm_expr_t *make_binary(arena_t *arena, arvm_binop_t op, arvm_expr_t *lhs,
                         arvm_expr_t *rhs);

arvm_expr_t *make_in_interval(arena_t *arena, arvm_expr_t *value,
                              arvm_val_t min, arvm_val_t max);

arvm_expr_t *make_ref(arena_t *arena, arvm_ref_t ref);

arvm_expr_t *make_call(arena_t *arena, arvm_func_t *target, arvm_expr_t *arg);

arvm_expr_t *make_const(arena_t *arena, arvm_val_t value);

void clone_expr(arena_t *arena, const arvm_expr_t *src, arvm_expr_t *dst);