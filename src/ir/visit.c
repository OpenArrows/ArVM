#include "visit.h"
#include "ir.h"

void arvm_visit_children(arvm_expr_t expr, arvm_visitor visitor, void *ctx) {
  switch (expr->kind) {
  case NARY:
    for (size_t i = 0; i < expr->nary.operands.size; i++)
      arvm_visit(expr->nary.operands.exprs[i], visitor, ctx);
    break;
  default:
    break;
  }
}

void arvm_visit(arvm_expr_t expr, arvm_visitor visitor, void *ctx) {
  arvm_visit_children(expr, visitor, ctx);
  visitor(expr, ctx);
}