#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_make_nary(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t const1 = {CONST, .const_ = {1}};
  arvm_expr_t const2 = {CONST, .const_ = {2}};
  arvm_expr_t nary = {
      NARY, .nary = {ADD, {3, (arvm_expr_t *[]){&const0, &const1, &const2}}}};
  TEST_ASSERT(is_identical(
      &nary, make_nary(&arena, ADD, 3, &const0, &const1, &const2)));
}

void test_make_in_interval(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t in_interval = {
      IN_INTERVAL,
      .in_interval = {&const0, ARVM_NEGATIVE_INFINITY, ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(is_identical(
      &in_interval, make_in_interval(&arena, &const0, ARVM_NEGATIVE_INFINITY,
                                     ARVM_POSITIVE_INFINITY)));
}

void test_make_arg_ref(void) {
  arvm_expr_t arg_ref = {ARG_REF};
  TEST_ASSERT(is_identical(&arg_ref, make_arg_ref(&arena)));
}

void test_make_call(void) {
  arvm_expr_t const0 = {CONST, .const_ = {0}};
  arvm_expr_t call = {CALL, .call = {NULL, &const0}};
  TEST_ASSERT(is_identical(&call, make_call(&arena, NULL, &const0)));
}

void test_make_const(void) {
  arvm_expr_t const_ = {CONST, .const_ = {ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(
      is_identical(&const_, make_const(&arena, ARVM_POSITIVE_INFINITY)));
}

void test_copy_expr(void) {
  arvm_expr_t arg_ref = {ARG_REF};
  arvm_expr_t lhs = {CONST, .const_ = {1}};
  arvm_expr_t rhs = {CALL, .call = {NULL, &arg_ref}};
  arvm_expr_t nary = {NARY, .nary = {OR, {2, (arvm_expr_t *[]){&lhs, &rhs}}}};
  arvm_expr_t copy = {NONE};
  copy_expr(&arena, &nary, &copy);
  TEST_ASSERT(is_identical(&copy, &nary));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_make_nary);
  RUN_TEST(test_make_in_interval);
  RUN_TEST(test_make_arg_ref);
  RUN_TEST(test_make_call);
  RUN_TEST(test_make_const);
  RUN_TEST(test_copy_expr);
  return UNITY_END();
}
