#include <../src/analyze.h>
#include <../src/builder.h>
#include <arvm.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

void test_make_binary(void) {
  struct arvm_expr const0 = {CONST, .const_ = {0}};
  struct arvm_expr const1 = {CONST, .const_ = {1}};
  struct arvm_expr binary = {BINARY,
                             .binary = {ARVM_BINARY_MOD, &const0, &const1}};
  TEST_ASSERT(arvm_is_identical(
      &binary, arvm_new_binary(&arena, ARVM_BINARY_MOD, &const0, &const1)));
}

void test_make_nary(void) {
  struct arvm_expr const0 = {CONST, .const_ = {0}};
  struct arvm_expr const1 = {CONST, .const_ = {1}};
  struct arvm_expr const2 = {CONST, .const_ = {2}};
  struct arvm_expr nary = {
      NARY,
      .nary = {ARVM_NARY_ADD, {3, (arvm_expr_t[]){&const0, &const1, &const2}}}};
  TEST_ASSERT(
      arvm_is_identical(&nary, arvm_new_nary(&arena, ARVM_NARY_ADD, 3, &const0,
                                             &const1, &const2)));
}

void test_make_in_interval(void) {
  struct arvm_expr const0 = {CONST, .const_ = {0}};
  struct arvm_expr in_interval = {
      IN_INTERVAL,
      .in_interval = {&const0, ARVM_NEGATIVE_INFINITY, ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(arvm_is_identical(&in_interval,
                                arvm_new_in_interval(&arena, &const0,
                                                     ARVM_NEGATIVE_INFINITY,
                                                     ARVM_POSITIVE_INFINITY)));
}

void test_make_arg_ref(void) {
  struct arvm_expr arg_ref = {ARG_REF};
  TEST_ASSERT(arvm_is_identical(&arg_ref, arvm_new_arg_ref(&arena)));
}

void test_make_call(void) {
  struct arvm_expr const0 = {CONST, .const_ = {0}};
  struct arvm_expr call = {CALL, .call = {NULL, &const0}};
  TEST_ASSERT(arvm_is_identical(&call, arvm_new_call(&arena, NULL, &const0)));
}

void test_make_const(void) {
  struct arvm_expr const_ = {CONST, .const_ = {ARVM_POSITIVE_INFINITY}};
  TEST_ASSERT(arvm_is_identical(
      &const_, arvm_new_const(&arena, ARVM_POSITIVE_INFINITY)));
}

void test_copy_expr(void) {
  struct arvm_expr arg_ref = {ARG_REF};
  struct arvm_expr lhs = {CONST, .const_ = {1}};
  struct arvm_expr rhs = {CALL, .call = {NULL, &arg_ref}};
  struct arvm_expr nary = {
      NARY, .nary = {ARVM_NARY_OR, {2, (arvm_expr_t[]){&lhs, &rhs}}}};
  struct arvm_expr copy = {NONE};
  arvm_copy_expr(&arena, &nary, &copy);
  TEST_ASSERT(arvm_is_identical(&copy, &nary));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_make_binary);
  RUN_TEST(test_make_nary);
  RUN_TEST(test_make_in_interval);
  RUN_TEST(test_make_arg_ref);
  RUN_TEST(test_make_call);
  RUN_TEST(test_make_const);
  RUN_TEST(test_copy_expr);
  return UNITY_END();
}
