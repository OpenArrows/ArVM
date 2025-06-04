#include "transform.h"
#include "builder.h"
#include <string.h>

void transpose(arena_t *arena, const arvm_expr_t *what, arvm_expr_t *where) {
  clone_expr(arena, what, where);
}

void nary_remove(arvm_expr_t *nary, arvm_expr_t *arg) {
  for (int i = 0; i < nary->nary.args.size; i++) {
    if (nary->nary.args.exprs[i] == arg) {
      nary->nary.args.size--;
      memcpy(&nary->nary.args.exprs[i], &nary->nary.args.exprs[i + 1],
             sizeof(arvm_expr_t *) * (nary->nary.args.size - i));
      break;
    }
  }
}