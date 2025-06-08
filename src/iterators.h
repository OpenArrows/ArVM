#include "iter.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct permutation_iterator_state {
  void **array;
  size_t length;
  size_t permutation_length;
  size_t *cycles;
} perm_iter_t;

// TODO: inline permutation iterator into n-ary matcher

bool permutation(perm_iter_t *it);