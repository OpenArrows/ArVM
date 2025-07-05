#ifndef UTIL_H
#define UTIL_H

#include "arvm.h"

arvm_int_t arvm_min(arvm_int_t a, arvm_int_t b);

arvm_int_t arvm_max(arvm_int_t a, arvm_int_t b);

arvm_int_t arvm_add(arvm_int_t a, arvm_int_t b);

arvm_int_t arvm_sub(arvm_int_t a, arvm_int_t b);

arvm_int_t arvm_pow2(arvm_int_t x);

arvm_int_t arvm_log2(arvm_int_t x);

arvm_int_t arvm_ones(arvm_int_t x);

#endif /* UTIL_H */