#include "arvm.h"

void visit(arvm_expr_t *expr, void (*visitor)(arvm_expr_t *, void *),
           void *ctx);

void visit_children(arvm_expr_t *expr, void (*visitor)(arvm_expr_t *, void *),
                    void *ctx);