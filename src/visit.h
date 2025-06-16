#ifndef VISIT_H
#define VISIT_H

#include "arvm.h"

typedef void (*arvm_visitor)(arvm_expr_t, void *);

void arvm_visit(arvm_expr_t expr, arvm_visitor visitor, void *ctx);

void arvm_visit_children(arvm_expr_t expr, arvm_visitor visitor, void *ctx);

#endif /* VISIT_H */