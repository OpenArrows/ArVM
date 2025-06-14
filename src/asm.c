#include "asm.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ASM_LINK_GROUP_SIZE 8
#define ASM_STACK_GROUP_SIZE 4

void *asm_ptr(asm_t *asm_, asm_label_t label) {
  if (!asm_->buf->xptr)
    return NULL;
  return &asm_->buf->xptr[label.offset];
}

static void asm_link_label(asm_t *asm_, asm_label_t *label, asm_dword_t *ptr) {
  asm_dword_t offset = (asm_->buf->offset - asm_->buf->tail->header.offset) +
                       ((char *)ptr - asm_->buf->tail->data);
  if (label->resolved) {
    *ptr = offset - label->offset;
    return;
  }
  if (++label->links.count > label->links.capacity) {
    label->links.capacity += ASM_LINK_GROUP_SIZE;
    label->links.links = realloc(label->links.links, sizeof(asm_label_link_t) *
                                                         label->links.capacity);
  }
  asm_label_link_t *link = &label->links.links[label->links.count - 1];
  link->ptr = ptr;
  link->offset = offset;
}

void asm_label(asm_t *asm_, asm_label_t *label) {
  label->offset = asm_->buf->offset;
  label->resolved = true;
  for (size_t i = 0; i < label->links.count; i++) {
    asm_label_link_t link = label->links.links[i];
    *link.ptr = label->offset - link.offset;
  }
  free(label->links.links);
}

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
void asm_x86_emit_rex(asm_t *asm_, bool w, bool r, bool x, bool b) {
  asm_byte_t rex = 0x40 | (w << 3) | (r << 2) | (x << 1) | b;
  xbuf_write(asm_->buf, &rex, sizeof(rex));
}

void asm_x86_emit_imm64(asm_t *asm_, asm_qword_t imm) {
  xbuf_write(asm_->buf, &imm, sizeof(imm));
}
#endif

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
void asm_x86_emit_opcode(asm_t *asm_, asm_byte_t op) {
  xbuf_write(asm_->buf, &op, sizeof(op));
}

void asm_x86_emit_modrm(asm_t *asm_, asm_byte_t mod, asm_reg_t reg,
                        asm_reg_t rm) {
  asm_byte_t modrm = (mod << 6) | ((reg & 0x7) << 3) | (rm & 0x7);
  xbuf_write(asm_->buf, &modrm, sizeof(modrm));
}

void asm_x86_emit_rel32(asm_t *asm_, asm_label_t *label) {
  asm_dword_t reserved = 0;
  asm_dword_t *rel32 = xbuf_write(asm_->buf, &reserved, sizeof(reserved));
  asm_link_label(asm_, label, rel32);
}

void asm_x86_emit_imm8(asm_t *asm_, asm_byte_t imm) {
  xbuf_write(asm_->buf, &imm, sizeof(imm));
}

void asm_x86_emit_imm32(asm_t *asm_, asm_dword_t imm) {
  xbuf_write(asm_->buf, &imm, sizeof(imm));
}

void asm_x86_emit_mov_r32_imm32(asm_t *asm_, asm_reg_t reg, asm_dword_t imm) {
  asm_x86_emit_opcode(asm_, 0xB8 | (reg & 0x7));
  asm_x86_emit_imm32(asm_, imm);
}

void asm_x86_emit_mov_r32_r32(asm_t *asm_, asm_reg_t dst, asm_reg_t src) {
  asm_x86_emit_opcode(asm_, 0x89);
  asm_x86_emit_modrm(asm_, 0b11, src, dst);
}
#endif

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
void asm_x86_emit_mov_r64_imm32(asm_t *asm_, asm_reg_t reg, asm_dword_t imm) {
  asm_x86_emit_rex(asm_, true, reg >= R8, false, false);
  asm_x86_emit_opcode(asm_, 0xC7);
  asm_x86_emit_modrm(asm_, 0b11, 0, reg);
  asm_x86_emit_imm32(asm_, imm);
}

void asm_x86_emit_mov_r64_imm64(asm_t *asm_, asm_reg_t reg, asm_qword_t imm) {
  if (imm >> 32 == 0)
    return asm_x86_emit_mov_r64_imm32(asm_, reg, (asm_dword_t)imm);
  asm_x86_emit_rex(asm_, true, reg >= R8, false, false);
  asm_x86_emit_opcode(asm_, 0xB8 | (reg & 0x7));
  asm_x86_emit_imm64(asm_, imm);
}

void asm_x86_emit_mov_r64_r64(asm_t *asm_, asm_reg_t dst, asm_reg_t src) {
  asm_x86_emit_rex(asm_, true, src >= R8, dst >= R8, false);
  asm_x86_emit_mov_r32_r32(asm_, dst, src);
}
#endif

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86
void asm_x86_mov_op(asm_t *asm_, asm_reg_t dst, asm_operand_t op) {
  switch (op.type) {
  case ARG:
    // TODO
    break;
  case REG:
    asm_x86_emit_mov_r32_r32(asm_, dst, op.reg);
    break;
  case IMM:
    asm_x86_emit_mov_r32_imm32(asm_, dst, op.imm);
    break;
  default:
    unreachable();
  }
}
#elif ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
void asm_x86_mov_op(asm_t *asm_, asm_reg_t dst, asm_operand_t op) {
  switch (op.type) {
  case ARG:
    asm_x86_emit_mov_r64_r64(asm_, dst, RCX);
    break;
  case REG:
    asm_x86_emit_mov_r64_r64(asm_, dst, op.reg);
    break;
  case IMM:
    asm_x86_emit_mov_r64_imm64(asm_, dst, op.imm);
    break;
  default:
    unreachable();
  }
}
#endif

void asm_arg(asm_t *asm_) { asm_push_op(asm_, (asm_operand_t){ARG}); }

void asm_const(asm_t *asm_, asm_maxword_t value) {
  asm_push_op(asm_, (asm_operand_t){IMM, .imm = value});
}

void asm_mod(asm_t *asm_, asm_maxword_t divisor) {
  asm_operand_t divident = asm_pop_op(asm_);
  asm_free_op(asm_, divident);

  if (divident.type == IMM) {
    asm_push_op(asm_, (asm_operand_t){IMM, .imm = divident.imm % divisor});
    return;
  }

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86
  // TODO
#elif ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  asm_reserve_reg(asm_, RAX);
  asm_reserve_reg(asm_, RDX);

  asm_x86_mov_op(asm_, RAX, divident);

  asm_x86_emit_rex(asm_, true, false, false, false);
  asm_x86_emit_opcode(asm_, 0x31);
  asm_x86_emit_modrm(asm_, 0b11, RDX, RDX);

  asm_x86_emit_mov_r64_imm64(asm_, RBX, divisor);

  asm_x86_emit_rex(asm_, true, false, false, false);
  asm_x86_emit_opcode(asm_, 0xF7);
  asm_x86_emit_modrm(asm_, 0b11, 6, RBX);

  asm_->allocated_registers[RAX] = false;

  asm_push_op(asm_, (asm_operand_t){REG, .reg = RDX});
#endif
}

void asm_add(asm_t *asm_) {
  asm_operand_t op2 = asm_pop_op(asm_);
  asm_operand_t op1 = asm_pop_op(asm_);
  asm_free_op(asm_, op1);
  asm_free_op(asm_, op2);

  if (op1.type == IMM && op2.type == IMM) {
    asm_push_op(asm_, (asm_operand_t){IMM, .imm = op1.imm + op2.imm});
    return;
  }

  if (op2.type == REG && op1.type != REG) {
    asm_operand_t tmp = op2;
    op2 = op1;
    op1 = tmp;
  }

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  if ((op2.type == IMM && op2.imm >> 32 != 0) ||
      (op1.type == IMM && op1.imm >> 8 == 0)) {
    asm_operand_t tmp = op2;
    op2 = op1;
    op1 = tmp;
  }
#endif

  asm_operand_t result = op1.type == REG ? op1 : asm_alloc_op(asm_);
  asm_push_op(asm_, result);

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  if (op1.type != REG)
    asm_x86_mov_op(asm_, result.reg, op1);

  switch (op2.type) {
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  case ARG:
    // ADD r/m16/32/64, r16/32/64
    asm_x86_emit_rex(asm_, true, false, result.reg >= R8, false);
    asm_x86_emit_opcode(asm_, 0x01);
    asm_x86_emit_modrm(asm_, 0b11, RCX, result.reg);
    break;
  case REG:
    asm_x86_emit_rex(asm_, true, op2.reg >= R8, result.reg >= R8, false);
    asm_x86_emit_opcode(asm_, 0x01);
    asm_x86_emit_modrm(asm_, 0b11, op2.reg, result.reg);
    break;
  case IMM:
    asm_x86_emit_rex(asm_, true, false, result.reg >= R8, false);
    if (op2.imm >> 8 == 0 && false) {
      // ADD r/m16/32/64, imm8
      asm_x86_emit_opcode(asm_, 0x83);
      asm_x86_emit_modrm(asm_, 0b11, 0, result.reg);
      asm_x86_emit_imm8(asm_, (asm_byte_t)op2.imm);
    } else {
      // ADD r/m16/32/64, imm16/32/64
      asm_x86_emit_opcode(asm_, 0x81);
      asm_x86_emit_modrm(asm_, 0b11, 0, result.reg);
      asm_x86_emit_imm32(asm_, op2.imm);
    }
    break;
#else
  case ARG:
    // TODO
    break;
  case REG:
    asm_x86_emit_opcode(asm_, 0x01);
    asm_x86_emit_modrm(asm_, 0b11, op2.reg, result.reg);
    break;
  case IMM:
    if (op2.imm >> 8 == 0) {
      // ADD r/m16/32, imm8
      asm_x86_emit_opcode(asm_, 0x83);
      asm_x86_emit_modrm(asm_, 0b11, 0, result.reg);
      asm_x86_emit_imm8(asm_, (asm_byte_t)op2.imm);
    } else {
      // ADD r/m16/32, imm16/32
      asm_x86_asm_x86_emit_opcode(asm_, 0x81);
      asm_x86_emit_modrm(asm_, 0b11, 0, result.reg);
      asm_x86_emit_imm32(asm_, op2.imm);
    }
    break;
#endif
  default:
    unreachable();
  }
#else
  unreachable();
#endif
}

void asm_ret(asm_t *asm_) {
  assert(asm_->operand_stack.count == 1);
  asm_operand_t op = asm_pop_op(asm_);
  asm_free_op(asm_, op);

#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  asm_x86_mov_op(asm_, RAX, op);
#else
  asm_x86_mov_op(asm_, EAX, op);
#endif
  asm_x86_emit_opcode(asm_, 0xC3); // RET
#else
  unreachable();
#endif
}

void asm_build(asm_t *asm_) {
  xbuf_map(asm_->buf);
  xbuf_free(asm_->buf);
  memset(&asm_->allocated_registers, 0, sizeof(asm_->allocated_registers));
  free(asm_->operand_stack.operands);
  asm_->operand_stack.count = 0;
  asm_->operand_stack.stack_size = 0;
}

void asm_free(asm_t *asm_) { xbuf_unmap(asm_->buf); }

void asm_reserve_reg(asm_t *asm_, asm_reg_t reg) {
  if (!asm_->allocated_registers[reg])
    goto success;
  for (size_t i = asm_->operand_stack.count; i-- > 0;) {
    asm_operand_t *other = &asm_->operand_stack.operands[i];
    if (other->type == REG && !other->spilled && other->reg == reg) {
      asm_spill(asm_, other);
      goto success;
    }
  }
  return;
success:
  asm_->allocated_registers[reg] = true;
}

asm_operand_t asm_alloc_op(asm_t *asm_) {
  asm_operand_t op = {};
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  op.type = REG;
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  for (asm_reg_t reg = RAX + 1; reg <= R15; reg++)
    if (reg != RCX)
#else
  for (asm_reg_t reg = EAX + 1; reg <= EDX; reg++)
#endif
    {
      if (!asm_->allocated_registers[reg]) {
        op.reg = reg;
        goto allocated;
      }
    }
  for (size_t i = asm_->operand_stack.count; i-- > 0;) {
    asm_operand_t *other = &asm_->operand_stack.operands[i];
    if (other->type == REG && !other->spilled) {
      asm_spill(asm_, other);
      op.reg = other->reg;
      goto allocated;
    }
  }
  unreachable();
allocated:
  asm_->allocated_registers[op.reg] = true;
#else
  unreachable();
#endif
  return op;
}

void asm_free_op(asm_t *asm_, asm_operand_t op) {
  switch (op.type) {
  case REG:
    asm_->allocated_registers[op.reg] = false;
    break;
  default:
    break;
  }
}

void asm_spill(asm_t *asm_, asm_operand_t *op) {
  assert(op->type == REG);
  assert(!op->spilled);
  op->spilled = true;
  asm_->allocated_registers[op->reg] = false;
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  // PUSH r16/32
  asm_x86_emit_opcode(asm_, 0x50 | op->reg);
#else
  unreachable();
#endif
}

void asm_unspill(asm_t *asm_, asm_operand_t *op) {
  assert(op->type == REG);
  assert(op->spilled);
  assert(!asm_->allocated_registers[op->reg]);
  op->spilled = false;
  asm_->allocated_registers[op->reg] = true;
#if ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86 || ARVM_JIT_ARCH == ARVM_JIT_ARCH_X86_64
  // POP r16/32
  asm_x86_emit_opcode(asm_, 0x58 | op->reg);
#else
  unreachable();
#endif
}

void asm_push_op(asm_t *asm_, asm_operand_t op) {
  asm_->operand_stack.count++;
  if (asm_->operand_stack.count > asm_->operand_stack.stack_size) {
    asm_->operand_stack.stack_size += ASM_STACK_GROUP_SIZE;
    asm_->operand_stack.operands =
        realloc(asm_->operand_stack.operands,
                sizeof(asm_operand_t) * asm_->operand_stack.stack_size);
  }
  asm_->operand_stack.operands[asm_->operand_stack.count - 1] = op;
}

asm_operand_t asm_pop_op(asm_t *asm_) {
  assert(asm_->operand_stack.count > 0);
  asm_operand_t op = asm_->operand_stack.operands[--asm_->operand_stack.count];
  if (op.type == REG && op.spilled)
    asm_unspill(asm_, &op);
  return op;
}