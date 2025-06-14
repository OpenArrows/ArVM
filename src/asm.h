#include "platform.h"
#include "xbuf.h"
#include <inttypes.h>
#include <stdbool.h>

typedef uint8_t asm_byte_t;
typedef uint16_t asm_word_t;
typedef uint32_t asm_dword_t;
typedef uint64_t asm_qword_t;
typedef uintmax_t asm_maxword_t;

typedef struct asm_label_link {
  asm_dword_t *ptr;
  asm_dword_t offset;
} asm_label_link_t;

typedef struct asm_label {
  bool resolved;
  asm_dword_t offset;
  struct {
    asm_label_link_t *links;
    size_t count;
    size_t capacity;
  } links;
} asm_label_t;

typedef enum asm_operand_type { ARG, REG, IMM } asm_operand_type_t;

typedef enum asm_register {
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86
  EAX,
  ECX,
  EDX,
  EBX,
#elif ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  RAX,
  RCX,
  RDX,
  RBX,
  RSP,
  RBP,
  RSI,
  RDI,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
#endif

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  SIB = 0b100,
#endif
} asm_reg_t;

typedef struct asm_operand {
  asm_operand_type_t type;
  union {
    asm_maxword_t imm;
    asm_reg_t reg;
  };
  bool spilled;
} asm_operand_t;

typedef struct asm_state {
  xbuf_t *buf;

  bool allocated_registers[
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86
      4
#elif ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
      16
#endif
  ];

  struct {
    asm_operand_t *operands;
    size_t count;
    size_t stack_size;
  } operand_stack;
} asm_t;

void *asm_ptr(asm_t *asm_, asm_label_t label);

void asm_label(asm_t *asm_, asm_label_t *label);

asm_operand_t asm_alloc_op(asm_t *asm_);

void asm_free_op(asm_t *asm_, asm_operand_t op);

void asm_spill(asm_t *asm_, asm_operand_t *op);

void asm_unspill(asm_t *asm_, asm_operand_t *op);

void asm_push_op(asm_t *asm_, asm_operand_t op);

asm_operand_t asm_pop_op(asm_t *asm_);

void asm_arg(asm_t *asm_);

void asm_const(asm_t *asm_, asm_maxword_t value);

void asm_add(asm_t *asm_);

void asm_ret(asm_t *asm_);

void asm_build(asm_t *asm_);

void asm_free(asm_t *asm_);