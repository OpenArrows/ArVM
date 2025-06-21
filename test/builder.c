#include "util/macros.h"
#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_new_range(void) {
  TEST_ASSERT(arvm_is_identical(arvm_new_range(&arena, 0, 10),
                                &(struct arvm_expr){RANGE, .range = {0, 10}}));
}

void test_new_modeq(void) {
  TEST_ASSERT(arvm_is_identical(arvm_new_modeq(&arena, 2, 1),
                                &(struct arvm_expr){MODEQ, .modeq = {2, 1}}));
}

void test_new_nary(void) {
  TEST_ASSERT(arvm_is_identical(
      arvm_new_nary(&arena, ARVM_OP_XOR, 2,
                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                    &(struct arvm_expr){RANGE, .range = {1, 2}}),
      &(struct arvm_expr){
          NARY,
          .nary = {ARVM_OP_XOR,
                   .operands = {
                       2, (arvm_expr_t[]){
                              &(struct arvm_expr){RANGE, .range = {0, 1}},
                              &(struct arvm_expr){RANGE, .range = {1, 2}}}}}}));
}

void test_new_call(void) {
  struct arvm_func func = {NULL};
  TEST_ASSERT(arvm_is_identical(arvm_new_call(&arena, &func, 1),
                                &(struct arvm_expr){CALL, .call = {&func, 1}}));
}

void test_clone(void) {
  arvm_expr_t exprs[] = {
      arvm_new_range(&arena, 0, 10),
      arvm_new_modeq(&arena, 2, 1),
      arvm_new_nary(&arena, ARVM_OP_XOR, 2,
                    &(struct arvm_expr){RANGE, .range = {0, 1}},
                    &(struct arvm_expr){RANGE, .range = {1, 2}}),
      arvm_new_call(&arena, &(struct arvm_func){NULL}, 1),
  };

  for (size_t i = 0; i < lengthof(exprs); i++)
    TEST_ASSERT(arvm_is_identical(arvm_clone(&arena, exprs[i]), exprs[i]));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_new_range);
  RUN_TEST(test_new_modeq);
  RUN_TEST(test_new_nary);
  RUN_TEST(test_new_call);
  RUN_TEST(test_clone);
  return UNITY_END();
}
