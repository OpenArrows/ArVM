#ifndef ARVM_H
#define ARVM_H

#include <stdint.h>

typedef uintmax_t arvm_val_t;

#define ARVM_FALSE 0
#define ARVM_TRUE 1

#define ARVM_INFINITY UINTMAX_MAX

typedef struct arvm_expr *arvm_expr_t;

typedef struct arvm_func *arvm_func_t;

arvm_func_t arvm_create_function(arvm_expr_t value);

void arvm_set_function_value(arvm_func_t func, arvm_expr_t value);

void arvm_set_function_name(arvm_func_t func, const char *name);

void arvm_print_function(arvm_func_t func);

void arvm_build_function(arvm_func_t func);

arvm_val_t arvm_call_function(arvm_func_t func, arvm_val_t arg);

typedef enum arvm_nary_op {
  ARVM_OP_OR,
  ARVM_OP_NOR,
  ARVM_OP_XOR,
  ARVM_OP_TH2, // TH2 (2-threshold) is true if at least two of the given
               // operands are true
} arvm_nary_op_t;

// Boolean n-ary expression
arvm_expr_t arvm_make_nary(arvm_nary_op_t op, size_t operand_count, ...);

arvm_expr_t arvm_make_nary_v(arvm_nary_op_t op, size_t operand_count,
                             va_list operands);

arvm_expr_t arvm_make_nary_p(arvm_nary_op_t op, size_t operand_count,
                             arvm_expr_t *operands);

// Checks if the argument is in given range
arvm_expr_t arvm_make_range(arvm_val_t min, arvm_val_t max);

// Checks if the argument's residue is equal to the given value when divided by
// given constant divisor
arvm_expr_t arvm_make_modeq(arvm_val_t divisor, arvm_val_t residue);

// Evaluates another ArVM function with the current argument value - constant
// offset
arvm_expr_t arvm_make_call(arvm_func_t func, arvm_val_t offset);

#endif /* ARVM_H */