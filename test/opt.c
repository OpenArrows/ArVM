#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/opt.h>
#include <unity.h>


static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_unwrap_nary(void) {
  struct arvm_func func = {
      arvm_new_nary(&arena, ARVM_NARY_OR, 1, arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_fold_nary(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_ADD, 3,
      arvm_new_nary(&arena, ARVM_NARY_ADD, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_expr(&arena, UNKNOWN)),
      arvm_new_expr(&arena, UNKNOWN),
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_expr(&arena, UNKNOWN)))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_nary(
      &arena, ARVM_NARY_ADD, 6, arvm_new_expr(&arena, UNKNOWN),
      arvm_new_expr(&arena, UNKNOWN), arvm_new_expr(&arena, UNKNOWN),
      arvm_new_expr(&arena, UNKNOWN), arvm_new_expr(&arena, UNKNOWN),
      arvm_new_expr(&arena, UNKNOWN));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_eval_nary(void) {
  struct arvm_func func = {
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_const(&arena, 1),
                    arvm_new_const(&arena, 2), arvm_new_const(&arena, 3))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_const(&arena, 6);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_normalize_interval(void) {
  struct arvm_func func = {arvm_new_in_interval(
      &arena,
      arvm_new_nary(&arena, ARVM_NARY_ADD, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, -2)),
      0, ARVM_POSITIVE_INFINITY)};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN),
                                         2, ARVM_POSITIVE_INFINITY);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_eval_interval(void) {
  struct arvm_func func = {
      arvm_new_in_interval(&arena, arvm_new_const(&arena, 1), 0, 3)};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_const(&arena, ARVM_TRUE);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_merge_intervals(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 5),
      arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 10);

  TEST_ASSERT(arvm_is_identical(func.value, exp));

  func = (struct arvm_func){arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 0, 5),
      arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_func(&func, &arena, &arena);

  exp = arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN), 3, 5);

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
      arvm_new_nary(&arena, ARVM_NARY_ADD, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, 1)))}; // TODO: maybe add a way to
                                                  // create distinct UNKNOWNs
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_nary(&arena, ARVM_NARY_ADD, 2,
                                  arvm_new_expr(&arena, UNKNOWN),
                                  arvm_new_const(&arena, 1))),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_arg_ref(&arena),
                    arvm_new_nary(&arena, ARVM_NARY_ADD, 2,
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
                    arvm_new_in_interval(&arena, arvm_new_expr(&arena, UNKNOWN),
                                         1, ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_const_step_recursion(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_in_interval(&arena, arvm_new_arg_ref(&arena), 1, 1),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_call(&arena, &func,
                                  arvm_new_nary(&arena, ARVM_NARY_ADD, 2,
                                                arvm_new_arg_ref(&arena),
                                                arvm_new_const(&arena, -2))),
                    arvm_new_in_interval(&arena, arvm_new_arg_ref(&arena), 1,
                                         ARVM_POSITIVE_INFINITY)))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_in_interval(&arena, arvm_new_arg_ref(&arena), 1,
                           ARVM_POSITIVE_INFINITY),
      arvm_new_in_interval(&arena,
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
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unwrap_nary);
  RUN_TEST(test_fold_nary);
  RUN_TEST(test_eval_nary);
  RUN_TEST(test_normalize_interval);
  RUN_TEST(test_eval_interval);
  RUN_TEST(test_merge_intervals);
  RUN_TEST(test_annulment_law);
  RUN_TEST(test_identity_law);
  RUN_TEST(test_idempotent_law);
  RUN_TEST(test_absorption_law);
  RUN_TEST(test_distributive_law);
  RUN_TEST(test_call_inlining);
  RUN_TEST(test_const_step_recursion);
  RUN_TEST(test_short_circuit);
  return UNITY_END();
}