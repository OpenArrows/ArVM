#include <arvm.h>
#include <assert.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/match.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

#define TEST_MATCHES(expr, pattern, captures, expected)                        \
  do {                                                                         \
    arvm_expr_t *_captures[] = {UNWRAP(captures)};                             \
    arvm_expr_t _expected[][lengthof(_captures)] = {UNWRAP(expected)};         \
    bool results[lengthof(_expected)] = {false};                               \
    FOR_EACH_MATCH(expr, pattern, {                                            \
      for (size_t j = 0; j < lengthof(_expected); j++) {                       \
        if (results[j])                                                        \
          continue;                                                            \
        for (size_t i = 0; i < lengthof(_captures); i++) {                     \
          if (*_captures[i] != _expected[j][i])                                \
            goto UNIQUE(_next);                                                \
        }                                                                      \
        results[j] = true;                                                     \
        goto UNIQUE(_found);                                                   \
        UNIQUE(_next) :;                                                       \
      }                                                                        \
      TEST_FAIL();                                                             \
      UNIQUE(_found) :;                                                        \
    });                                                                        \
    for (size_t i = 0; i < lengthof(results); i++)                             \
      TEST_ASSERT(results[i]);                                                 \
  } while (0)

#define TEST_DOES_MATCH(expr, pattern) TEST_ASSERT(matches(expr, pattern))

#define TEST_DOES_NOT_MATCH(expr, pattern)                                     \
  TEST_ASSERT_FALSE(matches(expr, pattern))

void test_match_anyval(void) { TEST_ASSERT(val_matches(123, ANYVAL())); }

void test_match_slotval(void) {
  TEST_ASSERT(val_matches(123, SLOTVAL()));

  arvm_expr_t range1 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t range2 = arvm_new_range(&arena, 0, 2);
  arvm_expr_t range3 = arvm_new_range(&arena, 1, 2);
  arvm_expr_t nary =
      arvm_new_nary(&arena, ARVM_OP_OR, 3, range1, range2, range3);

  arvm_expr_t cap_range1, cap_range2;
  TEST_MATCHES(nary,
               NARY(ANYVAL(), RANGE_AS(cap_range1, SLOTVAL(), ANYVAL()),
                    RANGE_AS(cap_range2, ANYVAL(), SLOTVAL())),
               (&cap_range1, &cap_range2), ({range3, range1}));
}

void test_match_val(void) {
  TEST_ASSERT(val_matches(123, VAL(123)));
  TEST_ASSERT_FALSE(val_matches(124, VAL(123)));
}

void test_match_rangeval(void) {
  TEST_ASSERT_FALSE(val_matches(3, RANGEVAL(5, 10)));
  TEST_ASSERT(val_matches(5, RANGEVAL(5, 10)));
  TEST_ASSERT(val_matches(7, RANGEVAL(5, 10)));
  TEST_ASSERT(val_matches(10, RANGEVAL(5, 10)));
  TEST_ASSERT_FALSE(val_matches(12, RANGEVAL(5, 10)));
}

void test_match_any(void) {
  arvm_expr_t expr = arvm_new_range(&arena, 0, 1);

  arvm_expr_t cap_expr;
  TEST_MATCHES(expr, ANY_AS(cap_expr), (&cap_expr), ({expr}));
}

void test_match_slot(void) {
  arvm_expr_t expr = arvm_new_range(&arena, 0, 1);

  arvm_expr_t cap_expr;
  TEST_MATCHES(expr, SLOT_AS(cap_expr), (&cap_expr), ({expr}));

  arvm_expr_t range1 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t range2 = arvm_new_range(&arena, 1, 2);
  arvm_expr_t range3 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t nary =
      arvm_new_nary(&arena, ARVM_OP_OR, 3, range1, range2, range3);

  arvm_expr_t cap_slot1, cap_slot2;
  TEST_MATCHES(nary, NARY(ANYVAL(), SLOT_AS(cap_slot1), SLOT_AS(cap_slot2)),
               (&cap_slot1, &cap_slot2), ({range1, range3}, {range3, range1}));
}

void test_match_range(void) {
  arvm_expr_t expr = arvm_new_range(&arena, 0, 1);

  arvm_expr_t cap_expr;
  TEST_MATCHES(expr, RANGE_AS(cap_expr, VAL(0), VAL(1)), (&cap_expr), ({expr}));
  TEST_DOES_NOT_MATCH(expr, RANGE(VAL(1), VAL(0)));
}

void test_match_modeq(void) {
  arvm_expr_t expr = arvm_new_modeq(&arena, 2, 1);

  arvm_expr_t cap_expr;
  TEST_MATCHES(expr, MODEQ_AS(cap_expr, VAL(2), VAL(1)), (&cap_expr), ({expr}));
  TEST_DOES_NOT_MATCH(expr, MODEQ(VAL(3), VAL(1)));
}

void test_match_nary(void) {
  arvm_expr_t subnary_range1 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t subnary_range2 = arvm_new_range(&arena, 1, 10);
  arvm_expr_t subnary =
      arvm_new_nary(&arena, ARVM_OP_OR, 2, subnary_range1, subnary_range2);
  arvm_expr_t range = arvm_new_range(&arena, 1, 2);
  arvm_expr_t expr = arvm_new_nary(&arena, ARVM_OP_OR, 2, range, subnary);

  arvm_expr_t cap_expr, cap_subnary, cap_subnary_any, cap_any;
  TEST_MATCHES(expr,
               NARY_AS(cap_expr, VAL(ARVM_OP_OR),
                       NARY_AS(cap_subnary, ANYVAL(), ANY_AS(cap_subnary_any)),
                       ANY_AS(cap_any)),
               (&cap_expr, &cap_subnary, &cap_subnary_any, &cap_any),
               ({expr, subnary, subnary_range1, range},
                {expr, subnary, subnary_range2, range}));
  TEST_DOES_MATCH(expr, NARY(VAL(ARVM_OP_OR), ANY()));
  TEST_DOES_NOT_MATCH(expr, NARY(VAL(ARVM_OP_NOR), ANY()));
}

void test_match_nary_fixed(void) {
  arvm_expr_t subnary_range1 = arvm_new_range(&arena, 0, 1);
  arvm_expr_t subnary_range2 = arvm_new_range(&arena, 1, 10);
  arvm_expr_t subnary =
      arvm_new_nary(&arena, ARVM_OP_OR, 2, subnary_range1, subnary_range2);
  arvm_expr_t range = arvm_new_range(&arena, 1, 2);
  arvm_expr_t expr = arvm_new_nary(&arena, ARVM_OP_OR, 2, range, subnary);

  arvm_expr_t cap_expr, cap_subnary, cap_subnary_any, cap_any;
  TEST_MATCHES(expr,
               NARY_FIXED_AS(cap_expr, VAL(ARVM_OP_OR),
                             NARY_FIXED_AS(cap_subnary, ANYVAL(),
                                           ANY_AS(cap_subnary_any), ANY()),
                             ANY_AS(cap_any)),
               (&cap_expr, &cap_subnary, &cap_subnary_any, &cap_any),
               ({expr, subnary, subnary_range1, range},
                {expr, subnary, subnary_range2, range}));
  TEST_DOES_NOT_MATCH(expr, NARY_FIXED(VAL(ARVM_OP_OR), ANY()));
  TEST_DOES_NOT_MATCH(expr, NARY_FIXED(VAL(ARVM_OP_NOR), ANY(), ANY()));
}

void test_match_call(void) {
  struct arvm_func func = {NULL};

  arvm_expr_t expr = arvm_new_call(&arena, &func, 1);

  arvm_expr_t cap_expr;
  TEST_MATCHES(expr, CALL_AS(cap_expr, ANYVAL(), VAL(1)), (&cap_expr),
               ({expr}));
  TEST_DOES_NOT_MATCH(expr, CALL(VAL((arvm_val_t)NULL), VAL(1)));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_match_anyval);
  RUN_TEST(test_match_slotval);
  RUN_TEST(test_match_val);
  RUN_TEST(test_match_rangeval);
  RUN_TEST(test_match_any);
  RUN_TEST(test_match_slot);
  RUN_TEST(test_match_range);
  RUN_TEST(test_match_modeq);
  RUN_TEST(test_match_nary);
  RUN_TEST(test_match_nary_fixed);
  RUN_TEST(test_match_call);
  return UNITY_END();
}
