#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/transform.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_transpose(void) {
  arvm_expr_t exprs[] = {
      arvm_new_range(&arena, 0, 10),
      arvm_new_modeq(&arena, 2, 1),
      arvm_new_nary(&arena, ARVM_OP_XOR, 2, arvm_new_range(&arena, 0, 1),
                    arvm_new_range(&arena, 1, 2)),
      arvm_new_call(&arena, &(struct arvm_func){NULL}, 1),
  };

  for (size_t i = 0; i < lengthof(exprs); i++)
    for (size_t j = 0; j < lengthof(exprs); j++) {
      arvm_expr_t what = exprs[i];
      arvm_expr_t where = exprs[j];
      arvm_transpose(&arena, what, where);
      TEST_ASSERT(arvm_is_identical(where, what));
    }
}

void test_nary_remove(void) {
  arvm_expr_t operand = arvm_new_range(&arena, 1, 2);
  arvm_expr_t nary = arvm_new_nary(&arena, ARVM_OP_OR, 2,
                                   arvm_new_range(&arena, 0, 1), operand);
  arvm_nary_remove_operand(nary, operand);
  TEST_ASSERT(
      arvm_is_identical(nary, arvm_new_nary(&arena, ARVM_OP_OR, 1,
                                            arvm_new_range(&arena, 0, 1))));
}

void test_replace(void) {
  arvm_expr_t expr =
      arvm_new_nary(&arena, ARVM_OP_XOR, 4, arvm_new_range(&arena, 0, 1),
                    arvm_new_range(&arena, 1, 2), arvm_new_range(&arena, 0, 2),
                    arvm_new_modeq(&arena, 3, 2));
  arvm_replace(&arena, expr, RANGE(ANYVAL(), VAL(2)),
               arvm_new_range(&arena, 2, 3));
  TEST_ASSERT(arvm_is_identical(
      expr,
      arvm_new_nary(&arena, ARVM_OP_XOR, 4, arvm_new_range(&arena, 0, 1),
                    arvm_new_range(&arena, 2, 3), arvm_new_range(&arena, 2, 3),
                    arvm_new_modeq(&arena, 3, 2))));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_transpose);
  RUN_TEST(test_nary_remove);
  RUN_TEST(test_replace);
  return UNITY_END();
}