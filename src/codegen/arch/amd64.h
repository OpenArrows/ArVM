#ifndef AMD64_H
#define AMD64_H

#include "x86.h"

enum arvm_register {
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
};

#define ARVM_REG_COUNT 16

#endif /* AMD64 */