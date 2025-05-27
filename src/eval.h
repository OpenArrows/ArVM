#include "arvm.h"

typedef struct eval_context {
  arvm_val_t arg;
} arvm_ctx_t;

arvm_val_t eval_binary(arvm_binary_expr_t *expr, arvm_ctx_t ctx);

arvm_val_t eval_in_interval(arvm_in_interval_expr_t *expr, arvm_ctx_t ctx);

arvm_val_t eval_ref(arvm_ref_expr_t *expr, arvm_ctx_t ctx);

arvm_val_t eval_call(arvm_call_expr_t *expr, arvm_ctx_t ctx);

arvm_val_t eval_const(arvm_const_expr_t *expr);

arvm_val_t eval_expr(arvm_expr_t *expr, arvm_ctx_t ctx);

arvm_val_t eval(arvm_func_t *func, arvm_val_t arg);