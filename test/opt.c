#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <opt.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_unwrap_nary(void) {
  arvm_func_t func = {make_nary(&arena, OR, 1, make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_fold_nary(void) {
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

void test_eval_nary(void) {
  arvm_func_t func = {make_nary(&arena, ADD, 3, make_const(&arena, 1),
                                make_const(&arena, 2), make_const(&arena, 3))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, 6);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_normalize_interval(void) {
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

void test_eval_interval(void) {
  arvm_func_t func = {make_in_interval(&arena, make_const(&arena, 1), 0, 3)};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, ARVM_TRUE);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_merge_intervals(void) {
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

void test_annulment_law(void) {
  arvm_func_t func = {make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                                make_const(&arena, 0))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, 0);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_identity_law(void) {
  arvm_func_t func = {make_nary(&arena, OR, 2, make_expr(&arena, UNKNOWN),
                                make_const(&arena, 0))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_idempotent_law(void) {
  arvm_func_t func = {make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                                make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_absorption_law(void) {
  arvm_func_t func = {
      make_nary(&arena, AND, 2,
                make_nary(&arena, OR, 2, make_expr(&arena, UNKNOWN),
                          make_arg_ref(&arena)),
                make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_distributive_law(void) {
  arvm_func_t func = {make_nary(
      &arena, AND, 2,
      make_nary(&arena, OR, 2, make_expr(&arena, UNKNOWN),
                make_arg_ref(&arena)),
      make_nary(
          &arena, ADD, 2, make_expr(&arena, UNKNOWN),
          make_const(&arena,
                     1)))}; // TODO: maybe add a way to create distinct UNKNOWNs
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_nary(&arena, OR, 2,
                make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                          make_nary(&arena, ADD, 2, make_expr(&arena, UNKNOWN),
                                    make_const(&arena, 1))),
                make_nary(&arena, AND, 2, make_arg_ref(&arena),
                          make_nary(&arena, ADD, 2, make_expr(&arena, UNKNOWN),
                                    make_const(&arena, 1))));

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_call_inlining(void) {
  arvm_func_t callee = {make_arg_ref(&arena)};

  arvm_func_t func = {make_call(&arena, &callee, make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_nary(&arena, AND, 2, make_expr(&arena, UNKNOWN),
                make_in_interval(&arena, make_expr(&arena, UNKNOWN), 1,
                                 ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_const_step_recursion(void) {
  arvm_func_t func = {make_nary(
      &arena, OR, 2, make_in_interval(&arena, make_arg_ref(&arena), 1, 1),
      make_nary(&arena, AND, 2,
                make_call(&arena, &func,
                          make_nary(&arena, ADD, 2, make_arg_ref(&arena),
                                    make_const(&arena, -2))),
                make_in_interval(&arena, make_arg_ref(&arena), 1,
                                 ARVM_POSITIVE_INFINITY)))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_nary(
      &arena, AND, 2,
      make_in_interval(&arena, make_arg_ref(&arena), 1, ARVM_POSITIVE_INFINITY),
      make_in_interval(
          &arena,
          make_binary(&arena, MOD, make_arg_ref(&arena), make_const(&arena, 2)),
          1, 1));

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_short_circuit(void) {
  arvm_func_t func = {make_nary(
      &arena, AND, 2, make_call(&arena, NULL, make_expr(&arena, UNKNOWN)),
      make_expr(&arena, UNKNOWN))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_call(&arena, NULL, make_expr(&arena, UNKNOWN));

  TEST_ASSERT(is_identical(func.value->nary.args.exprs[1], exp));
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