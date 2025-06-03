#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <opt.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_unwrap_nary() {
  arvm_func_t func = {make_nary(&arena, OR, 1, make_arg_ref(&arena))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp = make_arg_ref(&arena);

  TEST_ASSERT(is_identical(func.value, exp));
}

void test_fold_nary() {
  arvm_func_t func = {
      make_nary(&arena, ADD, 3,
                make_nary(&arena, ADD, 2, make_none(&arena), make_none(&arena)),
                make_const(&arena, 2),
                make_nary(&arena, ADD, 3, make_none(&arena), make_none(&arena),
                          make_none(&arena)))};
  arvm_optimize_fn(&func, &arena);

  arvm_expr_t *exp =
      make_nary(&arena, ADD, 6, make_none(&arena), make_none(&arena),
                make_const(&arena, 2), make_none(&arena), make_none(&arena),
                make_none(&arena));

  TEST_ASSERT(is_identical(func.value, exp));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unwrap_nary);
  RUN_TEST(test_fold_nary);
  return UNITY_END();
}