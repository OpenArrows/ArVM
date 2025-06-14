#include "codegen.h"
#include "visit.h"

static void codegen_visitor(arvm_expr_t *expr, void *ctx) {

}

void *arvm_codegen(arvm_func_t *func, arena_t *arena) {
  visit(func->value, codegen_visitor, arena);
}