#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <transform.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_transpose(void) {
  arvm_expr_t *what = make_expr(&arena, UNKNOWN);
  arvm_expr_t *where = make_nary(&arena, ADD, 1, what);
  transpose(&arena, what, where);
  TEST_ASSERT(is_identical(where, what));
}

void test_nary_remove(void) {
  arvm_expr_t *arg = make_expr(&arena, UNKNOWN);
  arvm_expr_t *nary = make_nary(&arena, ADD, 3, make_expr(&arena, UNKNOWN),
                                make_const(&arena, 1), arg);
  nary_remove(nary, arg);
  TEST_ASSERT(
      is_identical(nary, make_nary(&arena, ADD, 2, make_expr(&arena, UNKNOWN),
                                   make_const(&arena, 1))));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_transpose);
  RUN_TEST(test_nary_remove);
  return UNITY_END();
}