#include <analyze.h>
#include <arvm.h>
#include <builder.h>
#include <match.h>
#include <unity.h>

static arena_t arena = {sizeof(arvm_expr_t) * 16};

void setUp(void) {}

void tearDown(void) { arena_free(&arena); }

void test_match_anyval(void) {
  arvm_val_t val = 123;

  TEST_ASSERT(val_matches(val, ANYVAL()));
}

void test_match_slotval(void) {
  arvm_expr_t *expr = make_nary(&arena, ADD, 3, make_const(&arena, 1),
                                make_const(&arena, 1), make_const(&arena, 2));

  TEST_ASSERT(
      matches(expr, NARY(ANYVAL(), CONST(SLOTVAL()), CONST(SLOTVAL()))));
}

void test_match_val(void) {
  arvm_val_t val = 123;

  TEST_ASSERT(val_matches(val, VAL(123)));
  TEST_ASSERT_FALSE(val_matches(val, VAL(124)));
}

void test_match_rangeval(void) {
  arvm_val_t val = 123;

  TEST_ASSERT(val_matches(val, RANGEVAL(120, 125)));
  TEST_ASSERT_FALSE(val_matches(val, RANGEVAL(130, 135)));
}

void test_match_capture(void) {
  arvm_expr_t *expr = make_expr(&arena, UNKNOWN);

  arvm_expr_t *match;
  TEST_ASSERT(matches(expr, ANY_AS(match)));
  TEST_ASSERT_EQUAL(expr, match);
}

void test_match_any(void) {
  arvm_expr_t *expr = make_expr(&arena, UNKNOWN);

  TEST_ASSERT(matches(expr, ANY()));
}

void test_match_slot(void) {
  arvm_expr_t *unk1 = make_expr(&arena, UNKNOWN);
  arvm_expr_t *unk2 = make_expr(&arena, UNKNOWN);
  arvm_expr_t *expr = make_nary(
      &arena, ADD, 3, make_nary(&arena, ADD, 1, make_const(&arena, 1)),
      make_nary(&arena, ADD, 1, unk1), unk2);

  arvm_expr_t *match;
  TEST_ASSERT(
      matches(expr, NARY(ANYVAL(), NARY(VAL(ADD), SLOT_AS(match)), SLOT())));

  TEST_ASSERT(match == unk1 || match == unk2);

  TEST_ASSERT_FALSE(
      matches(expr, NARY(ANYVAL(), SLOT_AS(match), SLOT(), SLOT())));
}

void test_match_binary(void) {
  arvm_expr_t *lhs = make_const(&arena, 0);
  arvm_expr_t *rhs = make_const(&arena, 1);
  arvm_expr_t *expr = make_binary(&arena, MOD, lhs, rhs);

  TEST_ASSERT(matches(expr, BINARY(VAL(MOD), CONST(VAL(0)), CONST(VAL(1)))));
}

void test_match_nary(void) {
  arvm_expr_t *arg1 = make_const(&arena, 0);
  arvm_expr_t *arg2 = make_const(&arena, 1);
  arvm_expr_t *expr = make_nary(&arena, OR, 2, arg1, arg2);

  TEST_ASSERT(matches(expr, NARY(VAL(OR), CONST(VAL(0)))));
  TEST_ASSERT_FALSE(matches(expr, NARY(VAL(AND), CONST(VAL(0)))));
  TEST_ASSERT_FALSE(matches(
      expr, NARY(ANYVAL(), CONST(VAL(0)), CONST(VAL(1)), CONST(VAL(2)))));
}

void test_match_nary_fixed(void) {
  arvm_expr_t *arg1 = make_const(&arena, 0);
  arvm_expr_t *arg2 = make_const(&arena, 1);
  arvm_expr_t *expr = make_nary(&arena, OR, 2, arg1, arg2);

  TEST_ASSERT(matches(expr, NARY_FIXED(VAL(OR), CONST(VAL(0)), CONST(VAL(1)))));
  TEST_ASSERT_FALSE(matches(expr, NARY_FIXED(VAL(OR), CONST(VAL(0)))));
  TEST_ASSERT_FALSE(
      matches(expr, NARY_FIXED(VAL(AND), CONST(VAL(0)), CONST(VAL(1)))));
}

void test_match_in_interval(void) {
  arvm_expr_t *value = make_expr(&arena, UNKNOWN);
  arvm_expr_t *expr = make_in_interval(&arena, value, -10, 10);

  TEST_ASSERT(matches(expr, IN_INTERVAL(ANY(), VAL(-10), VAL(10))));
}

void test_match_arg_ref(void) {
  arvm_expr_t *expr = make_arg_ref(&arena);

  TEST_ASSERT(matches(expr, ARG_REF()));
}

void test_match_call(void) {
  arvm_expr_t *arg = make_expr(&arena, UNKNOWN);
  arvm_expr_t *expr = make_call(&arena, NULL, arg);

  TEST_ASSERT(matches(expr, CALL(VAL((arvm_val_t)NULL), ANY())));
}

void test_match_const(void) {
  arvm_expr_t *expr = make_const(&arena, 1);

  TEST_ASSERT(matches(expr, CONST(ANYVAL())));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_match_anyval);
  RUN_TEST(test_match_slotval);
  RUN_TEST(test_match_val);
  RUN_TEST(test_match_rangeval);
  RUN_TEST(test_match_capture);
  RUN_TEST(test_match_any);
  RUN_TEST(test_match_slot);
  RUN_TEST(test_match_binary);
  RUN_TEST(test_match_nary);
  RUN_TEST(test_match_nary_fixed);
  RUN_TEST(test_match_in_interval);
  RUN_TEST(test_match_arg_ref);
  RUN_TEST(test_match_call);
  RUN_TEST(test_match_const);
  return UNITY_END();
}
