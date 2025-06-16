#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/transform.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_transpose(void) {
  arvm_expr_t what = arvm_new_expr(&arena, UNKNOWN);
  arvm_expr_t where = arvm_new_nary(&arena, ARVM_NARY_ADD, 1, what);
  arvm_transpose(&arena, what, where);
  TEST_ASSERT(arvm_is_identical(where, what));
}

void test_nary_remove(void) {
  arvm_expr_t operand = arvm_new_expr(&arena, UNKNOWN);
  arvm_expr_t nary =
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, 1), operand);
  arvm_nary_remove_operand(nary, operand);
  TEST_ASSERT(
      arvm_is_identical(nary, arvm_new_nary(&arena, ARVM_NARY_ADD, 2,
                                            arvm_new_expr(&arena, UNKNOWN),
                                            arvm_new_const(&arena, 1))));
}

void test_replace(void) {
  arvm_expr_t expr =
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_const(&arena, 1),
                    arvm_new_call(&arena, NULL, arvm_new_const(&arena, 1)),
                    arvm_new_const(&arena, 3));
  arvm_replace(&arena, expr, CONST(VAL(1)), arvm_new_const(&arena, 2));
  TEST_ASSERT(arvm_is_identical(
      expr,
      arvm_new_nary(&arena, ARVM_NARY_ADD, 3, arvm_new_const(&arena, 2),
                    arvm_new_call(&arena, NULL, arvm_new_const(&arena, 2)),
                    arvm_new_const(&arena, 3))));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_transpose);
  RUN_TEST(test_nary_remove);
  RUN_TEST(test_replace);
  return UNITY_END();
}