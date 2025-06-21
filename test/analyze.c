#include <arvm.h>
#include <ir/analyze.h>
#include <ir/ir.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_is_identical(void) {
  TEST_ASSERT_FALSE(arvm_is_identical(NULL, NULL));

  TEST_ASSERT_FALSE(
      arvm_is_identical(&(struct arvm_expr){MODEQ, .modeq = {2, 0}},
                        &(struct arvm_expr){RANGE, .range = {2, 0}}));

  TEST_ASSERT(arvm_is_identical(&(struct arvm_expr){RANGE, .range = {0, 1}},
                                &(struct arvm_expr){RANGE, .range = {0, 1}}));
  TEST_ASSERT_FALSE(
      arvm_is_identical(&(struct arvm_expr){RANGE, .range = {0, 1}},
                        &(struct arvm_expr){RANGE, .range = {0, 2}}));
  TEST_ASSERT_FALSE(
      arvm_is_identical(&(struct arvm_expr){RANGE, .range = {0, 1}},
                        &(struct arvm_expr){RANGE, .range = {1, 1}}));

  TEST_ASSERT(arvm_is_identical(&(struct arvm_expr){MODEQ, .modeq = {2, 0}},
                                &(struct arvm_expr){MODEQ, .modeq = {2, 0}}));
  TEST_ASSERT_FALSE(
      arvm_is_identical(&(struct arvm_expr){MODEQ, .modeq = {2, 0}},
                        &(struct arvm_expr){MODEQ, .modeq = {2, 1}}));
  TEST_ASSERT_FALSE(
      arvm_is_identical(&(struct arvm_expr){MODEQ, .modeq = {2, 1}},
                        &(struct arvm_expr){MODEQ, .modeq = {3, 1}}));

  TEST_ASSERT(arvm_is_identical(
      &(struct arvm_expr){
          NARY,
          .nary = {ARVM_OP_OR,
                   .operands = {2,
                                (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                    &(struct arvm_expr){RANGE, .range = {1, 2}},
                                }}}},
      &(struct arvm_expr){
          NARY, .nary = {ARVM_OP_OR,
                         .operands = {
                             2, (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {1, 2}},
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                }}}}));
  TEST_ASSERT_FALSE(arvm_is_identical(
      &(struct arvm_expr){
          NARY,
          .nary = {ARVM_OP_OR,
                   .operands = {2,
                                (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                    &(struct arvm_expr){RANGE, .range = {1, 2}},
                                }}}},
      &(struct arvm_expr){
          NARY, .nary = {ARVM_OP_OR,
                         .operands = {
                             2, (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                    &(struct arvm_expr){RANGE, .range = {1, 3}},
                                }}}}));
  TEST_ASSERT_FALSE(arvm_is_identical(
      &(struct arvm_expr){
          NARY,
          .nary = {ARVM_OP_OR,
                   .operands = {2,
                                (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                    &(struct arvm_expr){RANGE, .range = {1, 2}},
                                }}}},
      &(struct arvm_expr){
          NARY, .nary = {ARVM_OP_NOR,
                         .operands = {
                             2, (arvm_expr_t[]){
                                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                                    &(struct arvm_expr){RANGE, .range = {1, 2}},
                                }}}}));

  {
    struct arvm_func func = {NULL};
    TEST_ASSERT(
        arvm_is_identical(&(struct arvm_expr){CALL, .call = {&func, 1}},
                          &(struct arvm_expr){CALL, .call = {&func, 1}}));
    TEST_ASSERT_FALSE(
        arvm_is_identical(&(struct arvm_expr){CALL, .call = {&func, 0}},
                          &(struct arvm_expr){CALL, .call = {&func, 1}}));
    TEST_ASSERT_FALSE(
        arvm_is_identical(&(struct arvm_expr){CALL, .call = {&func, 1}},
                          &(struct arvm_expr){CALL, .call = {NULL, 1}}));
  }
}

void test_ranges_overlap(void) {
  TEST_ASSERT(
      arvm_ranges_overlap(&(struct arvm_expr){RANGE, .range = {0, 10}},
                          &(struct arvm_expr){RANGE, .range = {5, 15}}));
  TEST_ASSERT(
      arvm_ranges_overlap(&(struct arvm_expr){RANGE, .range = {0, 10}},
                          &(struct arvm_expr){RANGE, .range = {10, 20}}));
  TEST_ASSERT(arvm_ranges_overlap(&(struct arvm_expr){RANGE, .range = {5, 10}},
                                  &(struct arvm_expr){RANGE, .range = {0, 5}}));
  TEST_ASSERT_FALSE(
      arvm_ranges_overlap(&(struct arvm_expr){RANGE, .range = {0, 10}},
                          &(struct arvm_expr){RANGE, .range = {15, 20}}));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_is_identical);
  RUN_TEST(test_ranges_overlap);
  return UNITY_END();
}
