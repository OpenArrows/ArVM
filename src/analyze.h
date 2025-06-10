#include "arvm.h"
#include <stdbool.h>

bool is_identical(const arvm_expr_t *a, const arvm_expr_t *b);

bool has_calls(const arvm_expr_t *expr);

bool intervals_overlap(const arvm_expr_t *a, const arvm_expr_t *b);