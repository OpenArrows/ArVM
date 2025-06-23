#include "arvm.h"
#include "eval.h"
#include "ir/builder.h"
#include "ir/ir.h"
#include "ir/opt.h"
#include "util/arena.h"
#include <stdarg.h>
#include <stdlib.h>

arvm_arena_t arvm_expr_arena = {sizeof(struct arvm_expr) * 2048};
arvm_arena_t arvm_opt_arena = {sizeof(struct arvm_expr) * 1024};
arvm_arena_t arvm_func_arena = {sizeof(struct arvm_func) * 256};

arvm_func_t arvm_create_function(arvm_expr_t value) {
  arvm_func_t func =
      arvm_arena_alloc(&arvm_func_arena, sizeof(struct arvm_func));
  *func = (struct arvm_func){value};
  return func;
}

void arvm_build_function(arvm_func_t func) {
  arvm_arena_t temp_arena = {sizeof(struct arvm_expr) * 4};
  arvm_optimize_func(func, &arvm_opt_arena, &temp_arena);
  arvm_arena_free(&temp_arena);
}

arvm_val_t arvm_call_function(arvm_func_t func, arvm_val_t arg) {
  return arvm_eval(func, arg);
}

arvm_expr_t arvm_make_nary(arvm_nary_op_t op, size_t operand_count, ...) {
  va_list args;
  va_start(args, operand_count);
  arvm_expr_t nary = arvm_new_nary_v(&arvm_expr_arena, op, operand_count, args);
  va_end(args);
  return nary;
}

arvm_expr_t arvm_make_range(arvm_val_t min, arvm_val_t max) {
  return arvm_new_range(&arvm_expr_arena, min, max);
}

arvm_expr_t arvm_make_modeq(arvm_val_t divisor, arvm_val_t residue) {
  return arvm_new_modeq(&arvm_expr_arena, divisor, residue);
}

arvm_expr_t arvm_make_call(arvm_func_t func, arvm_val_t offset) {
  return arvm_new_call(&arvm_expr_arena, func, offset);
}