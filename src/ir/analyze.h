#ifndef ANALYZE_H
#define ANALYZE_H

#include "arvm.h"
#include <stdbool.h>

bool arvm_is_identical(const arvm_expr_t a, const arvm_expr_t b);

bool arvm_has_calls(const arvm_expr_t expr);

bool arvm_intervals_overlap(const arvm_expr_t a, const arvm_expr_t b);

#endif /* ANALYZE_H */