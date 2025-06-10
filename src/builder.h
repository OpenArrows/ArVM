#include "arena.h"
#include "arvm.h"

arvm_expr_t *make_expr(arena_t *arena, arvm_expr_kind_t kind);

arvm_expr_t *make_binary(arena_t *arena, arvm_binary_op_t op, arvm_expr_t *lhs,
                         arvm_expr_t *rhs);

arvm_expr_t *make_nary(arena_t *arena, arvm_nary_op_t op, size_t arg_count,
                       ...);

arvm_expr_t *make_in_interval(arena_t *arena, arvm_expr_t *value,
                              arvm_val_t min, arvm_val_t max);

arvm_expr_t *make_arg_ref(arena_t *arena);

arvm_expr_t *make_call(arena_t *arena, arvm_func_t *target, arvm_expr_t *arg);

arvm_expr_t *make_const(arena_t *arena, arvm_val_t value);

arvm_expr_t *make_clone(arena_t *arena, const arvm_expr_t *expr);

void copy_expr(arena_t *arena, const arvm_expr_t *src, arvm_expr_t *dst);