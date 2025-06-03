#include <arvm.h>
#include <builder.h>
#include <eval.h>
#include <unity.h>

static arvm_ctx_t ctx = {1};
static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) {}

void test_eval_nary(void) {
  // TODO: test all ops

  arvm_expr_t *nary = make_nary(&arena, ADD, 3, make_const(&arena, 1),
                                make_const(&arena, 2), make_const(&arena, 3));
  TEST_ASSERT_EQUAL(6, eval_nary(&nary->nary, ctx));
}

void test_eval_in_interval(void) {
  arvm_expr_t *in_interval = make_in_interval(&arena, NULL, 5, 10);

  // 8 in [5; 10] == true
  in_interval->in_interval.value = make_const(&arena, 8);
  TEST_ASSERT_EQUAL(ARVM_TRUE,
                    eval_in_interval(&in_interval->in_interval, ctx));

  // 5 in [5; 10] == true
  in_interval->in_interval.value = make_const(&arena, 5);
  TEST_ASSERT_EQUAL(ARVM_TRUE,
                    eval_in_interval(&in_interval->in_interval, ctx));

  // 3 in [5; 10] == true
  in_interval->in_interval.value = make_const(&arena, 3);
  TEST_ASSERT_EQUAL(ARVM_FALSE,
                    eval_in_interval(&in_interval->in_interval, ctx));

  // 10 in [5; 10] == true
  in_interval->in_interval.value = make_const(&arena, 10);
  TEST_ASSERT_EQUAL(ARVM_TRUE,
                    eval_in_interval(&in_interval->in_interval, ctx));

  // 11 in [5; 10] == true
  in_interval->in_interval.value = make_const(&arena, 11);
  TEST_ASSERT_EQUAL(ARVM_FALSE,
                    eval_in_interval(&in_interval->in_interval, ctx));
}

void test_eval_call(void) {
  arvm_func_t func = {
      make_nary(&arena, ADD, 2, make_arg_ref(&arena), make_const(&arena, 1))};

  arvm_expr_t *call = make_call(&arena, &func, make_const(&arena, 2));
  TEST_ASSERT_EQUAL(3, eval_call(&call->call, ctx));
}

void test_eval_const(void) {
  arvm_expr_t *const_ = make_const(&arena, ARVM_POSITIVE_INFINITY);
  TEST_ASSERT_EQUAL(ARVM_POSITIVE_INFINITY, eval_const(&const_->const_));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_eval_nary);
  RUN_TEST(test_eval_in_interval);
  RUN_TEST(test_eval_call);
  RUN_TEST(test_eval_const);
  return UNITY_END();
}
