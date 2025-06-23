#include "print.h"
#include "ir.h"
#include <inttypes.h>
#include <stdio.h>

void arvm_print_expr(arvm_expr_t expr, FILE *file) {
  switch (expr->kind) {
  case RANGE:
    fprintf(file, "t in [%" PRIuMAX ", %" PRIuMAX "]", expr->range.min,
            expr->range.max);
    break;
  case MODEQ:
    fprintf(file, "t %% %" PRIuMAX " == %" PRIuMAX, expr->modeq.divisor,
            expr->modeq.residue);
    break;
  case NARY:
    switch (expr->nary.op) {
    case ARVM_OP_OR:
      fprintf(file, "OR");
      break;
    case ARVM_OP_NOR:
      fprintf(file, "NOR");
      break;
    case ARVM_OP_XOR:
      fprintf(file, "XOR");
      break;
    case ARVM_OP_TH2:
      fprintf(file, "TH2");
      break;
    }
    fputc('(', file);
    for (size_t i = 0; i < expr->nary.operands.size; i++) {
      arvm_print_expr(expr->nary.operands.exprs[i], file);
      if (i + 1 < expr->nary.operands.size)
        fprintf(file, ", ");
    }
    fputc(')', file);
    break;
  case CALL:
    fprintf(file, "%s(t - %" PRIuMAX ")", expr->call.func->name,
            expr->call.offset);
    break;
  }
}