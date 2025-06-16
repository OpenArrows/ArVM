#ifndef EVAL_H
#define EVAL_H

#include "arvm.h"

arvm_val_t arvm_eval_expr(arvm_expr_t expr);

arvm_val_t arvm_eval(arvm_func_t func, arvm_val_t arg);

#endif /* EVAL_H */