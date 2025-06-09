#include "arvm.h"
#include <stdbool.h>

bool is_identical(const arvm_expr_t *a, const arvm_expr_t *b);

bool intervals_overlap(const arvm_expr_t *a, const arvm_expr_t *b);