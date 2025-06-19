#ifndef BUILDER_H
#define BUILDER_H

#include "arvm.h"
#include "ir.h"
#include "util/arena.h"
#include <stdarg.h>

arvm_expr_t arvm_new_expr(arvm_arena_t *arena, arvm_expr_kind_t kind);

arvm_expr_t arvm_new_binary(arvm_arena_t *arena, arvm_binary_op_t op,
                            arvm_expr_t lhs, arvm_expr_t rhs);

arvm_expr_t arvm_new_nary_v(arvm_arena_t *arena, arvm_nary_op_t op,
                            size_t operand_count, va_list operands);

arvm_expr_t arvm_new_nary(arvm_arena_t *arena, arvm_nary_op_t op,
                          size_t operand_count, ...);

arvm_expr_t arvm_new_in_interval(arvm_arena_t *arena, arvm_expr_t value,
                                 arvm_val_t min, arvm_val_t max);

arvm_expr_t arvm_new_arg_ref(arvm_arena_t *arena);

arvm_expr_t arvm_new_call(arvm_arena_t *arena, arvm_func_t func,
                          arvm_expr_t arg);

arvm_expr_t arvm_new_const(arvm_arena_t *arena, arvm_val_t value);

arvm_expr_t arvm_clone(arvm_arena_t *arena, const arvm_expr_t expr);

void arvm_copy_expr(arvm_arena_t *arena, const arvm_expr_t src,
                    arvm_expr_t dst);

#endif /* BUILDER_H */