#include "imath.h"

arvm_val_t imin(arvm_val_t a, arvm_val_t b) { return a < b ? a : b; }

arvm_val_t imax(arvm_val_t a, arvm_val_t b) { return a > b ? a : b; }

arvm_val_t iadd(arvm_val_t a, arvm_val_t b) {
  if ((a == ARVM_POSITIVE_INFINITY && b == ARVM_NEGATIVE_INFINITY) ||
      (a == ARVM_NEGATIVE_INFINITY && b == ARVM_POSITIVE_INFINITY))
    return 0;
  if (a == ARVM_POSITIVE_INFINITY || b == ARVM_POSITIVE_INFINITY)
    return ARVM_POSITIVE_INFINITY;
  if (a == ARVM_NEGATIVE_INFINITY || b == ARVM_NEGATIVE_INFINITY)
    return ARVM_NEGATIVE_INFINITY;
  return a + b;
}