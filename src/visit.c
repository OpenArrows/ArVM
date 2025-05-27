#include "visit.h"

void visit_children(arvm_expr_t *expr, void (*visitor)(arvm_expr_t *, void *),
                    void *ctx) {
  switch (expr->kind) {
  case BINARY:
    visit(expr->binary.lhs, visitor, ctx);
    visit(expr->binary.rhs, visitor, ctx);
    break;
  case IN_INTERVAL:
    visit(expr->in_interval.value, visitor, ctx);
    break;
  case CALL:
    visit(expr->call.arg, visitor, ctx);
    break;
  default:
    break;
  }
}

void visit(arvm_expr_t *expr, void (*visitor)(arvm_expr_t *, void *),
           void *ctx) {
  visit_children(expr, visitor, ctx);
  visitor(expr, ctx);
}