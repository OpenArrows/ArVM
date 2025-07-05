#include "math.h"

arvm_int_t arvm_min(arvm_int_t a, arvm_int_t b) { return a < b ? a : b; }

arvm_int_t arvm_max(arvm_int_t a, arvm_int_t b) { return a > b ? a : b; }

arvm_int_t arvm_add(arvm_int_t a, arvm_int_t b) {
  return b > ARVM_INFINITY - a ? ARVM_INFINITY : a + b;
}

arvm_int_t arvm_sub(arvm_int_t a, arvm_int_t b) { return b > a ? 0 : a - b; }

arvm_int_t arvm_pow2(arvm_int_t x) { return 1 << x; }

arvm_int_t arvm_log2(arvm_int_t x) {
  arvm_int_t r = 0;
  while (x >>= 1)
    r++;
  return r;
}

arvm_int_t arvm_ones(arvm_int_t x) { return arvm_pow2(x) - 1; }
