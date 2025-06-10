#include <analyze.h>
#include <arvm.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_is_identical(void) {
  arvm_expr_t const_a = {CONST, .const_ = {0}};
  arvm_expr_t const_b = {CONST, .const_ = {1}};
  arvm_expr_t const_c = {CONST, .const_ = {1}};
  arvm_expr_t expr1 = {
      NARY, .nary = {ADD, {2, (arvm_expr_t *[]){&const_a, &const_b}}}};
  arvm_expr_t expr2 = {
      NARY, .nary = {ADD, {2, (arvm_expr_t *[]){&const_a, &const_c}}}};
  arvm_expr_t expr3 = {
      NARY, .nary = {ADD, {2, (arvm_expr_t *[]){&const_b, &const_c}}}};
  TEST_ASSERT(is_identical(&expr1, &expr2));
  TEST_ASSERT_FALSE(is_identical(&expr2, &expr3));
}

void test_has_calls(void) {
  arvm_expr_t const_ = {CONST, .const_ = {0}};
  arvm_expr_t call = {CALL, .call = {NULL, &const_}};
  arvm_expr_t expr1 = {NARY,
                       .nary = {ADD, {2, (arvm_expr_t *[]){&const_, &call}}}};
  arvm_expr_t expr2 = {NARY,
                       .nary = {ADD, {2, (arvm_expr_t *[]){&const_, &const_}}}};
  TEST_ASSERT(has_calls(&expr1));
  TEST_ASSERT_FALSE(has_calls(&expr2));
}

void test_intervals_overlap(void) {
  arvm_expr_t unk = {UNKNOWN};
  TEST_ASSERT(intervals_overlap(
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 5, 15}}));
  TEST_ASSERT(intervals_overlap(
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 10, 20}}));
  TEST_ASSERT(intervals_overlap(
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, -10, 0}}));
  TEST_ASSERT_FALSE(intervals_overlap(
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(arvm_expr_t){IN_INTERVAL, .in_interval = {&unk, 15, 20}}));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_is_identical);
  RUN_TEST(test_has_calls);
  RUN_TEST(test_intervals_overlap);
  return UNITY_END();
}
