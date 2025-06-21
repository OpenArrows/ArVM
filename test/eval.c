#include <arvm.h>
#include <eval.h>
#include <ir/builder.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_eval_range(void) {
  struct arvm_func func = {};

  func.value = arvm_new_range(&arena, 5, 10);
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 4));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 5));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 7));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 10));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 11));
}

void test_eval_modeq(void) {
  struct arvm_func func = {};

  func.value = arvm_new_modeq(&arena, 2, 0);
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 3));

  func.value = arvm_new_modeq(&arena, 2, 1);
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 3));

  func.value = arvm_new_modeq(&arena, 4, 3);
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 7));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 11));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 13));
}

void test_eval_nary(void) {
  struct arvm_func func = {};

  func.value =
      arvm_new_nary(&arena, ARVM_OP_OR, 2, arvm_new_range(&arena, 1, 2),
                    arvm_new_range(&arena, 2, 3));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 0));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 1));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 3));

  func.value =
      arvm_new_nary(&arena, ARVM_OP_NOR, 2, arvm_new_range(&arena, 1, 2),
                    arvm_new_range(&arena, 2, 3));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 0));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 1));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 3));

  func.value =
      arvm_new_nary(&arena, ARVM_OP_XOR, 2, arvm_new_range(&arena, 1, 2),
                    arvm_new_range(&arena, 2, 3));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 0));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 1));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 3));

  func.value =
      arvm_new_nary(&arena, ARVM_OP_TH2, 2, arvm_new_range(&arena, 1, 2),
                    arvm_new_range(&arena, 2, 3));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 0));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 1));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 3));
}

void test_eval_call(void) {
  struct arvm_func func;

  struct arvm_func callee = {arvm_new_range(&arena, 0, 1)};

  func.value = arvm_new_call(&arena, &callee, 1);
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 0));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 1));
  TEST_ASSERT_EQUAL(ARVM_TRUE, arvm_eval(&func, 2));
  TEST_ASSERT_EQUAL(ARVM_FALSE, arvm_eval(&func, 3));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_eval_range);
  RUN_TEST(test_eval_modeq);
  RUN_TEST(test_eval_nary);
  RUN_TEST(test_eval_call);
  return UNITY_END();
}
