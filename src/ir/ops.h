#ifndef OPS_H
#define OPS_H

#include "util/macros.h"

#define ARVM_OPS(x) x(ARVM_OP_OR) x(ARVM_OP_NOR) x(ARVM_OP_XOR) x(ARVM_OP_TH2)

#define ARVM_OPS_ASSOCIATIVE(x) x(ARVM_OP_OR) x(ARVM_OP_XOR)

#define ARVM_OPS_NON_ASSOCIATIVE(x) EXPAND(x(ARVM_OP_NOR) x(ARVM_OP_TH2))

#endif /* OPS_H */