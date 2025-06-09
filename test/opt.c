#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <opt.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_unwrap_nary() {
  arvm_func_t func = {make_nary(&arena, OR, 1, make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_fold_nary() {
  arvm_func_t func = {make_nary(
      &arena, ADD, 3,
      make_nary(&arena, ADD, 2, make_expr(&arena, UNKNOWN),
                make_expr(&arena, UNKNOWN)),
      make_expr(&arena, UNKNOWN),
      make_nary(&arena, ADD, 3, make_expr(&arena, UNKNOWN),
                make_expr(&arena, UNKNOWN), make_expr(&arena, UNKNOWN)))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_nary(
      &arena, ADD, 6, make_expr(&arena, UNKNOWN), make_expr(&arena, UNKNOWN),
      make_expr(&arena, UNKNOWN), make_expr(&arena, UNKNOWN),
      make_expr(&arena, UNKNOWN), make_expr(&arena, UNKNOWN));

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_eval_nary() {
  arvm_func_t func = {make_nary(&arena, ADD, 3, make_const(&arena, 1),
                                make_const(&arena, 2), make_const(&arena, 3))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, 6);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_normalize_interval() {
  arvm_func_t func = {
      make_in_interval(&arena,
                       make_nary(&arena, ADD, 2, make_expr(&arena, UNKNOWN),
                                 make_const(&arena, -2)),
                       0, ARVM_POSITIVE_INFINITY)};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_in_interval(&arena, make_expr(&arena, UNKNOWN), 2,
                                      ARVM_POSITIVE_INFINITY);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_eval_interval() {
  arvm_func_t func = {make_in_interval(&arena, make_const(&arena, 1), 0, 3)};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, ARVM_TRUE);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_merge_intervals() {
  arvm_func_t func = {make_nary(
      &arena, OR, 2, make_in_interval(&arena, make_expr(&arena, UNKNOWN), 0, 5),
      make_in_interval(&arena, make_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_in_interval(&arena, make_expr(&arena, UNKNOWN), 0, 10);

  TEST_ASSERT(is_identical(func.value, exp));

  func = (arvm_func_t){
      make_nary(&arena, AND, 2,
                make_in_interval(&arena, make_expr(&arena, UNKNOWN), 0, 5),
                make_in_interval(&arena, make_expr(&arena, UNKNOWN), 3, 10))};
  arvm_optimize_fn(&func, &arena);

  exp = make_in_interval(&arena, make_expr(&arena, UNKNOWN), 3, 5);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_annulment_law() {
  arvm_func_t func = {make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                                make_const(&arena, 0))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, 0);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_identity_law() {
  arvm_func_t func = {make_nary(&arena, OR, 2, make_expr(&arena, UNKNOWN),
                                make_const(&arena, 0))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_idempotent_law() {
  arvm_func_t func = {make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                                make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_call_inlining() {
  arvm_func_t callee = {make_arg_ref(&arena)};

  arvm_func_t func = {make_call(&arena, &callee, make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                make_in_interval(&arena, make_expr(&arena, UNKNOWN), 1,
                                 ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(is_identical(func.value, exp));
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
  RUN_TEST(test_call_inlining);
  return UNITY_END();
}