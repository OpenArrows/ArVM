#include <arvm.h>
#include <eval.h>
#include <unity.h>

static const arvm_ctx_t ctx = {1};

void setUp(void) {}

void tearDown(void) {}

void test_eval_binary(void) {
  arvm_expr_t const1 = {CONST, .const_ = {1}};
  arvm_expr_t const2 = {CONST, .const_ = {2}};
  arvm_expr_t binary = {BINARY, .binary = {ADD, &const1, &const2}};
  TEST_ASSERT_EQUAL(eval_binary(&binary.binary, ctx), 3);
}

void test_eval_in_interval(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t in_interval = {
      IN_INTERVAL,
      .in_interval = {&const0, ARVM_NEGATIVE_INFINITY, ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT_EQUAL(eval_in_interval(&in_interval.in_interval, ctx), ARVM_TRUE);
}

void test_eval_ref(void) {
  arvm_expr_t ref = {REF, .ref = {ARG}};
  TEST_ASSERT_EQUAL(eval_ref(&ref.ref, ctx), 1);
}

void test_eval_call(void) {
  arvm_expr_t const1 = {CONST, .const_ = {1}};
  arvm_expr_t const2 = {CONST, .const_ = {2}};

  arvm_expr_t arg = {REF, .ref = {ARG}};
  arvm_expr_t expr = {BINARY, .binary = {ADD, &arg, &const1}};
  arvm_func_t func = {&expr};

  arvm_expr_t call = {CALL, .call = {&func, &const2}};
  TEST_ASSERT_EQUAL(eval_call(&call.call, ctx), 3);
}

void test_eval_const(void) {
  arvm_expr_t const_ = {CONST, .const_ = {ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT_EQUAL(eval_const(&const_.const_), ARVM_POSITIVE_INFINITY);
}

void test_eval(void) {
  arvm_expr_t arg = {REF, .ref = {ARG}};
  arvm_expr_t const1 = {CONST, .const_ = {1}};
  arvm_expr_t expr = {BINARY, .binary = {ADD, &arg, &const1}};

  arvm_func_t func = {&expr};
  TEST_ASSERT_EQUAL(eval(&func, 4), 5);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_eval_binary);
  RUN_TEST(test_eval_in_interval);
  RUN_TEST(test_eval_ref);
  RUN_TEST(test_eval_call);
  RUN_TEST(test_eval_const);
  RUN_TEST(test_eval);
  return UNITY_END();
}
