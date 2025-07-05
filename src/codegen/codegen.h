#ifndef CODEGEN_H
#define CODEGEN_H

#include "platform.h"

#if CODEGEN_SUPPORTED

#include "arvm.h"
#include "util/arena.h"
#include "util/buffer.h"
#include <stdbool.h>
#include <stddef.h>

#if CODEGEN_ARCH == CODEGEN_ARCH_I386
#include "arch/i386.h"
#elif CODEGEN_ARCH == CODEGEN_ARCH_AMD64
#include "arch/amd64.h"
#else
#error Unknown architecture
#endif

typedef enum arvm_operand_type { OP_CONST, OP_REF, OP_ARG } arvm_operand_type_t;

typedef struct arvm_operand {
  arvm_operand_type_t type;
  union {
    arvm_int_t const_val;
    size_t ref_id;
  };
} arvm_operand_t;

typedef enum arvm_inst_type { INST_CONST, INST_ADD } arvm_inst_type_t;

typedef struct arvm_inst {
  arvm_inst_type_t type;
  arvm_operand_t result;
  union {
    struct {
      arvm_int_t value;
    } const_;
  };
} arvm_inst_t;

typedef struct arvm_native_func {
  BUFFER_T(arvm_inst_t) instructions;
  BUFFER_T(arvm_operand_t) operands;
  size_t ref_count;
  bool used_registers[ARVM_REG_COUNT];
} arvm_native_func_t;

arvm_native_func_t *arvm_gen_func(arvm_arena_t *arena);

void arvm_gen_const(arvm_native_func_t *func, arvm_int_t value);

#endif /* CODEGEN_SUPPORTED */

#endif /* CODEGEN_H */