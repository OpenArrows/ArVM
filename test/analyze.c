#include <arvm.h>
#include <ir/analyze.h>
#include <ir/ir.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_is_identical(void) {
  struct arvm_expr const_a = {CONST, .const_ = {0}};
  struct arvm_expr const_b = {CONST, .const_ = {1}};
  struct arvm_expr const_c = {CONST, .const_ = {1}};
  struct arvm_expr expr1 = {
      NARY, .nary = {ARVM_NARY_ADD, {2, (arvm_expr_t[]){&const_a, &const_b}}}};
  struct arvm_expr expr2 = {
      NARY, .nary = {ARVM_NARY_ADD, {2, (arvm_expr_t[]){&const_a, &const_c}}}};
  struct arvm_expr expr3 = {
      NARY, .nary = {ARVM_NARY_ADD, {2, (arvm_expr_t[]){&const_b, &const_c}}}};
  TEST_ASSERT(arvm_is_identical(&expr1, &expr2));
  TEST_ASSERT_FALSE(arvm_is_identical(&expr2, &expr3));
}

void test_has_calls(void) {
  struct arvm_expr const_ = {CONST, .const_ = {0}};
  struct arvm_expr call = {CALL, .call = {NULL, &const_}};
  struct arvm_expr expr1 = {
      NARY, .nary = {ARVM_NARY_ADD, {2, (arvm_expr_t[]){&const_, &call}}}};
  struct arvm_expr expr2 = {
      NARY, .nary = {ARVM_NARY_ADD, {2, (arvm_expr_t[]){&const_, &const_}}}};
  TEST_ASSERT(arvm_has_calls(&expr1));
  TEST_ASSERT_FALSE(arvm_has_calls(&expr2));
}

void test_intervals_overlap(void) {
  struct arvm_expr unk = {UNKNOWN};
  TEST_ASSERT(arvm_intervals_overlap(
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 5, 15}}));
  TEST_ASSERT(arvm_intervals_overlap(
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 10, 20}}));
  TEST_ASSERT(arvm_intervals_overlap(
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, -10, 0}}));
  TEST_ASSERT_FALSE(arvm_intervals_overlap(
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 0, 10}},
      &(struct arvm_expr){IN_INTERVAL, .in_interval = {&unk, 15, 20}}));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_is_identical);
  RUN_TEST(test_has_calls);
  RUN_TEST(test_intervals_overlap);
  return UNITY_END();
}
