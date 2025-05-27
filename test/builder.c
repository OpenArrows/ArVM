#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_make_binary(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t const1 = {CONST, .const_ = {1}};
  arvm_expr_t binary = {BINARY, .binary = {AND, &const0, &const1}};
  TEST_ASSERT(
      is_identical(&binary, make_binary(&arena, AND, &const0, &const1)));
}

void test_make_in_interval(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t in_interval = {
      IN_INTERVAL,
      .in_interval = {&const0, ARVM_NEGATIVE_INFINITY, ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(is_identical(
      &in_interval, make_in_interval(&arena, &const0, ARVM_NEGATIVE_INFINITY,
                                     ARVM_POSITIVE_INFINITY)));
}

void test_make_ref(void) {
  arvm_expr_t ref = {REF, .ref = {ARG}};
  TEST_ASSERT(is_identical(&ref, make_ref(&arena, ARG)));
}

void test_make_call(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t call = {CALL, .call = {NULL, &const0}};
  TEST_ASSERT(is_identical(&call, make_call(&arena, NULL, &const0)));
}

void test_make_const(void) {
  arvm_expr_t const_ = {CONST, .const_ = {ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(
      is_identical(&const_, make_const(&arena, ARVM_POSITIVE_INFINITY)));
}

void test_clone_expr(void) {
  arvm_expr_t arg = {REF, .ref = {ARG}};
  arvm_expr_t lhs = {CONST, .const_ = {1}};
  arvm_expr_t rhs = {CALL, .call = {NULL, &arg}};
  arvm_expr_t binary = {BINARY, .binary = {OR, &lhs, &rhs}};
  arvm_expr_t copy;
  clone_expr(&arena, &binary, &copy);
  TEST_ASSERT(is_identical(&copy, &binary));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_make_binary);
  RUN_TEST(test_make_in_interval);
  RUN_TEST(test_make_ref);
  RUN_TEST(test_make_call);
  RUN_TEST(test_make_const);
  RUN_TEST(test_clone_expr);
  return UNITY_END();
}
