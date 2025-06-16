#include <arvm.h>
#include <eval.h>
#include <ir/builder.h>
#include <unity.h>


static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_eval_binary(void) {
  arvm_expr_t binary =
      arvm_new_binary(&arena, ARVM_BINARY_MOD, arvm_new_const(&arena, 7),
                      arvm_new_const(&arena, 4));
  TEST_ASSERT_EQUAL(3, arvm_eval_expr(binary));
}

void test_eval_nary(void) {
  // TODO: test all ops

  arvm_expr_t nary =
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_const(&arena, 1),
                    arvm_new_const(&arena, 2), arvm_new_const(&arena, 3));
  TEST_ASSERT_EQUAL(6, arvm_eval_expr(nary));
}

void test_eval_in_interval(void) {
  arvm_expr_t in_interval = arvm_new_in_interval(&arena, NULL, 5, 10);

  // 8 in [5; 10] == true
  in_interval->in_interval.value = arvm_new_const(&arena, 8);
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval_expr(in_interval));

  // 5 in [5; 10] == true
  in_interval->in_interval.value = arvm_new_const(&arena, 5);
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval_expr(in_interval));

  // 3 in [5; 10] == true
  in_interval->in_interval.value = arvm_new_const(&arena, 3);
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval_expr(in_interval));

  // 10 in [5; 10] == true
  in_interval->in_interval.value = arvm_new_const(&arena, 10);
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval_expr(in_interval));

  // 11 in [5; 10] == true
  in_interval->in_interval.value = arvm_new_const(&arena, 11);
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval_expr(in_interval));
}

void test_eval_call(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_ADD, 2,
                                    arvm_new_arg_ref(&arena),
                                    arvm_new_const(&arena, 1))};

  arvm_expr_t call = arvm_new_call(&arena, &func, arvm_new_const(&arena, 2));
  TEST_ASSERT_EQUAL(3, arvm_eval_expr(call));
}

void test_eval_const(void) {
  arvm_expr_t const_ = arvm_new_const(&arena, ARVM_POSITIVE_INFINITY);
  TEST_ASSERT_EQUAL(ARVM_POSITIVE_INFINITY, arvm_eval_expr(const_));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_eval_binary);
  RUN_TEST(test_eval_nary);
  RUN_TEST(test_eval_in_interval);
  RUN_TEST(test_eval_call);
  RUN_TEST(test_eval_const);
  return UNITY_END();
}
