#include <analyze.h>
#include <arvm.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_is_identical(void) {
  arvm_expr_t const_a = {CONST, .const_ = {0}};
  arvm_expr_t const_b = {CONST, .const_ = {1}};
  arvm_expr_t const_c = {CONST, .const_ = {1}};
  arvm_expr_t expr1 = {BINARY, .binary = {ADD, &const_a, &const_b}};
  arvm_expr_t expr2 = {BINARY, .binary = {ADD, &const_a, &const_c}};
  arvm_expr_t expr3 = {BINARY, .binary = {ADD, &const_b, &const_c}};
  TEST_ASSERT(is_identical(&expr1, &expr2));
  TEST_ASSERT_FALSE(is_identical(&expr2, &expr3));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_is_identical);
  return UNITY_END();
}
