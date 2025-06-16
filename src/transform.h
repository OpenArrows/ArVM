#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "arena.h"
#include "arvm.h"
#include "match.h"

void arvm_transpose(arvm_arena_t *arena, const arvm_expr_t what,
                    arvm_expr_t where);

void arvm_nary_remove_operand(arvm_expr_t nary, arvm_expr_t operand);

void arvm_replace(arvm_arena_t *arena, arvm_expr_t where, pattern_t *what,
                  arvm_expr_t with);

#endif /* TRANSFORM_H */