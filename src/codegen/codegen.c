#include "codegen.h"
#include <stdlib.h>
#include <string.h>

arvm_native_func_t *arvm_gen_func(arvm_arena_t *arena) {
  arvm_native_func_t *func =
      arvm_arena_alloc(arena, sizeof(arvm_native_func_t));
  memset(func, 0, sizeof(*func));
  return func;
}

static void arvm_gen_inst(arvm_native_func_t *func, arvm_inst_t inst) {
  size_t i = func->instructions.count;
  BUFFER_GROW(func->instructions, 1);
  func->instructions.value[i] = inst;
}

static void arvm_push_op(arvm_native_func_t *func, arvm_operand_t op) {
  size_t i = func->operands.count;
  BUFFER_GROW(func->operands, 1);
  func->operands.value[i] = op;
}

void arvm_gen_const(arvm_native_func_t *func, arvm_val_t value) {
  arvm_operand_t result = {OP_CONST, .const_val = value};
  arvm_gen_inst(func, (arvm_inst_t){INST_CONST, .const_ = {value}});
  arvm_push_op(func, result);
}

void arvm_gen_add(arvm_native_func_t *func) {
  arvm_operand_t result = {OP_REF, .ref_id = func->ref_count++};
  arvm_gen_inst(func, (arvm_inst_t){INST_ADD, result});
  arvm_push_op(func, result);
}

void arvm_build_fn(arvm_native_func_t *func);