#include "arena.h"
#include "arvm.h"
#include "match.h"

void transpose(arena_t *arena, const arvm_expr_t *what, arvm_expr_t *where);

void nary_remove(arvm_expr_t *nary, arvm_expr_t *arg);

void replace(arena_t *arena, arvm_expr_t *where, pattern_t *what, arvm_expr_t *with);