#include "analyze.h"

bool is_identical(arvm_expr_t *a, arvm_expr_t *b) {
  if (a->kind != b->kind)
    return false;

  switch (a->kind) {
  case BINARY:
    return a->binary.op == b->binary.op &&
           is_identical(a->binary.lhs, b->binary.lhs) &&
           is_identical(a->binary.rhs, b->binary.rhs);
  case IN_INTERVAL:
    return a->in_interval.min == b->in_interval.min &&
           a->in_interval.max == b->in_interval.max &&
           is_identical(a->in_interval.value, b->in_interval.value);
  case REF:
    return a->ref.ref == b->ref.ref;
  case CALL:
    return a->call.target == b->call.target &&
           is_identical(a->call.arg, b->call.arg);
  case CONST:
    return a->const_.value == b->const_.value;
  case NONE:
    return true;
  }
}

bool is_symmetric(arvm_binop_t op) {
  switch (op) {
  case OR:
  case AND:
  case XOR:
  case ADD:
    return true;
  case MOD:
    return false;
  }
}