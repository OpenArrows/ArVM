#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/ops.h>
#include <ir/opt.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

#define TEST_OPT_EQUAL(expr, expected)                                         \
  do {                                                                         \
    struct arvm_func func = {expr};                                            \
    arvm_optimize_func(&func, &arena, &arena);                                 \
    TEST_ASSERT(arvm_is_identical(func.value, expected));                      \
  } while (0)

#define TEST_OPT(op) TEST_OPT_EQUAL(ORIGINAL(op), OPTIMIZED(op));

#define TEST_OPT_PRESERVED(op) TEST_OPT_EQUAL(ORIGINAL(op), ORIGINAL(op));

void test_unwrap_nary(void) {
#define ORIGINAL(op) arvm_new_nary(&arena, op, 1, arvm_new_range(&arena, 0, 1))
#define OPTIMIZED(op) arvm_new_range(&arena, 0, 1)

  ARVM_OPS_ASSOCIATIVE(TEST_OPT)
  ARVM_OPS_NON_ASSOCIATIVE(TEST_OPT_PRESERVED)

#undef ORIGINAL
#undef OPTIMIZED
}

void test_fold_nary(void) {
#define ORIGINAL(op)                                                           \
  arvm_new_nary(&arena, op, 3,                                                 \
                arvm_new_nary(&arena, op, 2, arvm_new_range(&arena, 0, 1),     \
                              arvm_new_range(&arena, 1, 2)),                   \
                arvm_new_nary(&arena,                                          \
                              op == ARVM_OP_OR ? ARVM_OP_XOR : ARVM_OP_OR, 2,  \
                              arvm_new_range(&arena, 2, 3),                    \
                              arvm_new_range(&arena, 3, 4)),                   \
                arvm_new_range(&arena, 4, 5))
#define OPTIMIZED(op)                                                          \
  arvm_new_nary(                                                               \
      &arena, op, 4, arvm_new_range(&arena, 0, 1),                             \
      arvm_new_range(&arena, 1, 2),                                            \
      arvm_new_nary(                                                           \
          &arena,                                                              \
          op == ARVM_OP_OR ? ARVM_OP_XOR                                       \
                           : ARVM_OP_OR /* only operations of                  \
                                           the same kind should be folded */   \
          ,                                                                    \
          2, arvm_new_range(&arena, 2, 3), arvm_new_range(&arena, 3, 4)),      \
      arvm_new_range(&arena, 4, 5))

  ARVM_OPS_ASSOCIATIVE(TEST_OPT)
  ARVM_OPS_NON_ASSOCIATIVE(TEST_OPT_PRESERVED)

#undef ORIGINAL
#undef OPTIMIZED
}

/*void test_normalize_interval(void) {
  struct arvm_func func = {arvm_new_range(
      &arena,
      arvm_new_nary(&arena, ARVM_NARY_TH2, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, -2)),
      0, ARVM_POSITIVE_INFINITY)};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 2,
                                   ARVM_POSITIVE_INFINITY);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_eval_interval(void) {
  struct arvm_func func = {
      arvm_new_range(&arena, arvm_new_const(&arena, 1), 0, 3)};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_const(&arena, ARVM_TRUE);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_merge_intervals(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 5),
      arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 10);

  TEST_ASSERT(arvm_is_identical(func.value, exp));

  func = (struct arvm_func){arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 5),
      arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_func(&func, &arena, &arena);

  exp = arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 5);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_annulment_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_const(&arena, 0))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_const(&arena, 0);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_identity_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_OR, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_const(&arena, 0))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_idempotent_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_absorption_law(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_nary(&arena, ARVM_NARY_OR, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_arg_ref(&arena)),
      arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_distributive_law(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_nary(&arena, ARVM_NARY_OR, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_arg_ref(&arena)),
      arvm_new_nary(&arena, ARVM_NARY_TH2, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, 1)))}; // TODO: maybe add a way to
                                                  // create distinct UNKNOWNs
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                  arvm_new_expr(&arena, UNKNOWN),
                                  arvm_new_const(&arena, 1))),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_arg_ref(&arena),
                    arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                  arvm_new_expr(&arena, UNKNOWN),
                                  arvm_new_const(&arena, 1))));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_call_inlining(void) {
  struct arvm_func callee = {arvm_new_arg_ref(&arena)};

  struct arvm_func func = {
      arvm_new_call(&arena, &callee, arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 1,
                                   ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_const_step_recursion(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1, 1),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_call(&arena, &func,
                                  arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                                arvm_new_arg_ref(&arena),
                                                arvm_new_const(&arena, -2))),
                    arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1,
                                   ARVM_POSITIVE_INFINITY)))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1,
                                   ARVM_POSITIVE_INFINITY),
                    arvm_new_range(&arena,
                                   arvm_new_binary(&arena, ARVM_BINARY_MOD,
                                                   arvm_new_arg_ref(&arena),
                                                   arvm_new_const(&arena, 2)),
                                   1, 1));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_short_circuit(void) {
  struct arvm_func func = {
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_call(&arena, NULL, arvm_new_expr(&arena, UNKNOWN)),
                    arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_call(&arena, NULL, arvm_new_expr(&arena, UNKNOWN));

  TEST_ASSERT(arvm_is_identical(func.value->nary.operands.exprs[1], exp));
}*/

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unwrap_nary);
  RUN_TEST(test_fold_nary);
  /*RUN_TEST(test_normalize_interval);
  RUN_TEST(test_eval_interval);
  RUN_TEST(test_merge_intervals);
  RUN_TEST(test_annulment_law);
  RUN_TEST(test_identity_law);
  RUN_TEST(test_idempotent_law);
  RUN_TEST(test_absorption_law);
  RUN_TEST(test_distributive_law);
  RUN_TEST(test_call_inlining);
  RUN_TEST(test_const_step_recursion);
  RUN_TEST(test_short_circuit);*/
  return UNITY_END();
}