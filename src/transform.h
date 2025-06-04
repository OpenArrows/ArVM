#include "arena.h"
#include "arvm.h"

void transpose(arena_t *arena, const arvm_expr_t *what, arvm_expr_t *where);

void nary_remove(arvm_expr_t *nary, arvm_expr_t *arg);