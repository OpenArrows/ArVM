#include "analyze.h"
#include <assert.h>
#include <string.h>

bool is_identical(const arvm_expr_t *a, const arvm_expr_t *b) {
  if (a == NULL || b == NULL)
    return false;

  if (a->kind != b->kind)
    return false;

  switch (a->kind) {
  case NARY: {
    if (a->nary.op != b->nary.op)
      return false;

    size_t argc = a->nary.args.size;
    if (b->nary.args.size != argc)
      return false;

    arvm_expr_t *args[argc];
    memcpy(args, a->nary.args.exprs, sizeof(args));
    for (int i = 0; i < argc; i++) {
      arvm_expr_t *arg_b = b->nary.args.exprs[i];
      for (int j = 0; j < argc; j++) {
        if (is_identical(args[j], arg_b)) {
          args[j] = NULL;
          goto next;
        }
      }
      return false;
    next:
      continue;
    }
    return true;
  }
  case IN_INTERVAL:
    return a->in_interval.min == b->in_interval.min &&
           a->in_interval.max == b->in_interval.max &&
           is_identical(a->in_interval.value, b->in_interval.value);
  case CALL:
    return a->call.target == b->call.target &&
           is_identical(a->call.arg, b->call.arg);
  case CONST:
    return a->const_.value == b->const_.value;
  case ARG_REF:
  case NONE:
  case UNKNOWN:
    return true;
  default:
    unreachable();
  }
}

bool intervals_overlap(const arvm_expr_t *a, const arvm_expr_t *b) {
  assert(a->kind == IN_INTERVAL && b->kind == IN_INTERVAL);
  return a->in_interval.min <= b->in_interval.max &&
         b->in_interval.min <= a->in_interval.max;
}