#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <opt.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

/*
// g(C1, C2) => C3
void test_opt_binary_const_const(void) {
  arvm_func_t func = {
      make_binary(&arena, ADD, make_const(&arena, 1), make_const(&arena, 2))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, 3);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x | true => true
void test_opt_any_or_true(void) {
  arvm_func_t func = {make_binary(&arena, OR, make_ref(&arena, ARG),
                                  make_const(&arena, ARVM_TRUE))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, ARVM_TRUE);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x | false => x
void test_opt_any_or_false(void) {
  arvm_func_t func = {make_binary(&arena, OR, make_ref(&arena, ARG),
                                  make_const(&arena, ARVM_FALSE))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_ref(&arena, ARG);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x & true => x
void test_opt_any_and_true(void) {
  arvm_func_t func = {make_binary(&arena, AND, make_ref(&arena, ARG),
                                  make_const(&arena, ARVM_TRUE))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_ref(&arena, ARG);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x & false => false
void test_opt_any_and_false(void) {
  arvm_func_t func = {make_binary(&arena, AND, make_ref(&arena, ARG),
                                  make_const(&arena, ARVM_FALSE))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_const(&arena, ARVM_FALSE);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x in X | x in Y => x in (X JOIN Y)
void test_opt_interval_or_interval(void) {
  arvm_func_t func = {make_binary(
      &arena, OR, make_in_interval(&arena, make_ref(&arena, ARG), 0, 10),
      make_in_interval(&arena, make_ref(&arena, ARG), 5, 20))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_in_interval(&arena, make_ref(&arena, ARG), 0, 20);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x in X & x in Y => x in (X INTERSECT Y)
void test_opt_interval_and_interval(void) {
  arvm_func_t func = {make_binary(
      &arena, AND, make_in_interval(&arena, make_ref(&arena, ARG), 0, 10),
      make_in_interval(&arena, make_ref(&arena, ARG), 5, 20))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_in_interval(&arena, make_ref(&arena, ARG), 5, 10);

  TEST_ASSERT(is_identical(func.value, exp));
}

// x + C1 + C2 => x1 + (C1 + C2)
void test_opt_any_plus_const_plus_const(void) {
  arvm_func_t func = {make_binary(
      &arena, ADD,
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, 1)),
      make_const(&arena, 2))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, 3));

  TEST_ASSERT(is_identical(func.value, exp));
}

// x + C in [A, B] => x in [A - C, B - C]
void test_opt_any_plus_const_in_interval(void) {
  arvm_func_t func = {make_in_interval(
      &arena,
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, -1)),
      0, 10)};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_in_interval(&arena, make_ref(&arena, ARG), 1, 11);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_opt_inline_call(void) {
  arvm_func_t f = {
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, -1))};

  arvm_func_t func = {make_call(
      &arena, &f,
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, -1)))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_binary(
      &arena, AND,
      make_binary(&arena, ADD, make_ref(&arena, ARG), make_const(&arena, -2)),
      make_in_interval(&arena, make_ref(&arena, ARG), 2,
                       ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(is_identical(func.value, exp));
}
*/

int main(void) {
  UNITY_BEGIN();
  /*
  RUN_TEST(test_opt_binary_const_const);
  RUN_TEST(test_opt_any_or_true);
  RUN_TEST(test_opt_any_or_false);
  RUN_TEST(test_opt_any_and_true);
  RUN_TEST(test_opt_any_and_false);
  RUN_TEST(test_opt_interval_or_interval);
  RUN_TEST(test_opt_interval_and_interval);
  RUN_TEST(test_opt_any_plus_const_plus_const);
  RUN_TEST(test_opt_any_plus_const_in_interval);
  RUN_TEST(test_opt_inline_call);
  */
  return UNITY_END();
}