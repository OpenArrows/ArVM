#include <arvm.h>
#include <ir/builder.h>
#include <ir/print.h>
#include <stdio.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

FILE *out;

#define TEST_ASSERT_OUT(expected)                                              \
  do {                                                                         \
    rewind(out);                                                               \
    char _buf[sizeof(expected)];                                               \
    TEST_ASSERT_NOT_NULL(fgets(_buf, sizeof(_buf), out));                      \
    TEST_ASSERT_EQUAL(EOF, fgetc(out));                                        \
    TEST_ASSERT_EQUAL_STRING(expected, _buf);                                  \
    rewind(out);                                                               \
  } while (0)

void setUp(void) { out = tmpfile(); }

void tearDown(void) {
  fclose(out);
  arvm_arena_free(&arena);
}

void test_print_range(void) {
  arvm_print_expr(arvm_new_range(&arena, 5, 10), out);
  TEST_ASSERT_OUT("t in [5, 10]");
}

void test_print_modeq(void) {
  arvm_print_expr(arvm_new_modeq(&arena, 2, 1), out);
  TEST_ASSERT_OUT("t % 2 == 1");
}

void test_print_nary(void) {
  arvm_expr_t op1 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t op2 = arvm_new_range(&arena, 0, 2);

  arvm_print_expr(arvm_new_nary(&arena, ARVM_OP_OR, 2, op1, op2), out);
  TEST_ASSERT_OUT("OR(t in [0, 1], t in [0, 2])");

  arvm_print_expr(arvm_new_nary(&arena, ARVM_OP_NOR, 2, op1, op2), out);
  TEST_ASSERT_OUT("NOR(t in [0, 1], t in [0, 2])");

  arvm_print_expr(arvm_new_nary(&arena, ARVM_OP_XOR, 2, op1, op2), out);
  TEST_ASSERT_OUT("XOR(t in [0, 1], t in [0, 2])");

  arvm_print_expr(arvm_new_nary(&arena, ARVM_OP_TH2, 2, op1, op2), out);
  TEST_ASSERT_OUT("TH2(t in [0, 1], t in [0, 2])");
}

void test_print_call(void) {
  struct arvm_func func = {.name = "f"};
  arvm_print_expr(arvm_new_call(&arena, &func, 1), out);
  TEST_ASSERT_OUT("f(t - 1)");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_print_range);
  RUN_TEST(test_print_modeq);
  RUN_TEST(test_print_nary);
  RUN_TEST(test_print_call);
  return UNITY_END();
}
