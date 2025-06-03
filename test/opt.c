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

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unwrap_nary);
  RUN_TEST(test_fold_nary);
  RUN_TEST(test_eval_nary);
  return UNITY_END();
}