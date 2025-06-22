#include <arvm.h>
#include <ir/analyze.h>
#include <ir/builder.h>
#include <ir/ops.h>
#include <ir/opt.h>
#include <unity.h>

static arvm_arena_t arena = {sizeof(struct arvm_expr) * 16};

void setUp(void) {}

void tearDown(void) { arvm_arena_free(&arena); }

#define TEST_OPT_EQUAL(expr, expected)                                         \
  do {                                                                         \
    struct arvm_func func = {expr};                                            \
    arvm_optimize_func(&func, &arena, &arena);                                 \
    TEST_ASSERT(arvm_is_identical(func.value, expected));                      \
  } while (0)

#define TEST_NARY_OPT(op) TEST_OPT_EQUAL(ORIGINAL(op), OPTIMIZED(op));

#define TEST_NARY_OPT_PRESERVED(op) TEST_OPT_EQUAL(ORIGINAL(op), ORIGINAL(op));

void test_unwrap_nary(void) {
#define ORIGINAL(op) arvm_new_nary(&arena, op, 1, arvm_new_range(&arena, 0, 1))
#define OPTIMIZED(op) arvm_new_range(&arena, 0, 1)

  ARVM_OPS_ASSOCIATIVE(TEST_NARY_OPT);
  ARVM_OPS_NON_ASSOCIATIVE(TEST_NARY_OPT_PRESERVED);

#undef ORIGINAL
#undef OPTIMIZED
}

void test_fold_nary(void) {
#define ORIGINAL(op)                                                           \
  arvm_new_nary(&arena, op, 3,                                                 \
                arvm_new_nary(&arena, op, 2, arvm_new_modeq(&arena, 2, 1),     \
                              arvm_new_modeq(&arena, 3, 2)),                   \
                arvm_new_nary(&arena,                                          \
                              op == ARVM_OP_OR ? ARVM_OP_XOR : ARVM_OP_OR, 2,  \
                              arvm_new_modeq(&arena, 5, 1),                    \
                              arvm_new_modeq(&arena, 4, 2)),                   \
                arvm_new_range(&arena, 0, 10))
#define OPTIMIZED(op)                                                          \
  arvm_new_nary(                                                               \
      &arena, op, 4, arvm_new_modeq(&arena, 2, 1),                             \
      arvm_new_modeq(&arena, 3, 2),                                            \
      arvm_new_nary(                                                           \
          &arena,                                                              \
          op == ARVM_OP_OR ? ARVM_OP_XOR                                       \
                           : ARVM_OP_OR /* only operations of                  \
                                           the same kind should be folded */   \
          ,                                                                    \
          2, arvm_new_modeq(&arena, 5, 1), arvm_new_modeq(&arena, 4, 2)),      \
      arvm_new_range(&arena, 0, 10))

  ARVM_OPS_ASSOCIATIVE(TEST_NARY_OPT);
  ARVM_OPS_NON_ASSOCIATIVE(TEST_NARY_OPT_PRESERVED);

#undef ORIGINAL
#undef OPTIMIZED
}

void test_union_ranges(void) {
#define ORIGINAL(op)                                                           \
  arvm_new_nary(&arena, op, 2, arvm_new_range(&arena, 0, 7),                   \
                arvm_new_range(&arena, 3, 10))
#define OPTIMIZED_ASSOCIATIVE(op) arvm_new_range(&arena, 0, 10)
#define OPTIMIZED_NON_ASSOCIATIVE(op)                                          \
  arvm_new_nary(&arena, op, 1, arvm_new_range(&arena, 0, 10))

#define OPTIMIZED OPTIMIZED_ASSOCIATIVE
  TEST_NARY_OPT(ARVM_OP_OR);
#undef OPTIMIZED
#define OPTIMIZED OPTIMIZED_NON_ASSOCIATIVE
  TEST_NARY_OPT(ARVM_OP_NOR);
  TEST_NARY_OPT_PRESERVED(ARVM_OP_XOR);
  TEST_NARY_OPT_PRESERVED(ARVM_OP_TH2);

#undef ORIGINAL
#undef OPTIMIZED_ASSOCIATIVE
#undef OPTIMIZED_NON_ASSOCIATIVE
#undef OPTIMIZED

  TEST_OPT_EQUAL(
      arvm_new_nary(&arena, ARVM_OP_OR, 2, arvm_new_range(&arena, 0, 4),
                    arvm_new_range(&arena, 5, 10)),
      arvm_new_nary(&arena, ARVM_OP_OR, 2, arvm_new_range(&arena, 0, 4),
                    arvm_new_range(&arena, 5, 10)));
}

/*void test_annulment_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_const(&arena, 0))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_const(&arena, 0);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_identity_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_OR, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_const(&arena, 0))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_idempotent_law(void) {
  struct arvm_func func = {arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                                         arvm_new_expr(&arena, UNKNOWN),
                                         arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_absorption_law(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_nary(&arena, ARVM_NARY_OR, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_arg_ref(&arena)),
      arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_expr(&arena, UNKNOWN);

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_distributive_law(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_AND, 2,
      arvm_new_nary(&arena, ARVM_NARY_OR, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_arg_ref(&arena)),
      arvm_new_nary(&arena, ARVM_NARY_TH2, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_const(&arena, 1)))}; // TODO: maybe add a way to
                                                  // create distinct UNKNOWNs
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                  arvm_new_expr(&arena, UNKNOWN),
                                  arvm_new_const(&arena, 1))),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_arg_ref(&arena),
                    arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                  arvm_new_expr(&arena, UNKNOWN),
                                  arvm_new_const(&arena, 1))));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_call_inlining(void) {
  struct arvm_func callee = {arvm_new_arg_ref(&arena)};

  struct arvm_func func = {
      arvm_new_call(&arena, &callee, arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_nary(&arena, ARVM_NARY_AND, 2, arvm_new_expr(&arena, UNKNOWN),
                    arvm_new_range(&arena, arvm_new_expr(&arena, UNKNOWN), 1,
                                   ARVM_POSITIVE_INFINITY));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_const_step_recursion(void) {
  struct arvm_func func = {arvm_new_nary(
      &arena, ARVM_NARY_OR, 2,
      arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1, 1),
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_call(&arena, &func,
                                  arvm_new_nary(&arena, ARVM_NARY_TH2, 2,
                                                arvm_new_arg_ref(&arena),
                                                arvm_new_const(&arena, -2))),
                    arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1,
                                   ARVM_POSITIVE_INFINITY)))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp =
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_range(&arena, arvm_new_arg_ref(&arena), 1,
                                   ARVM_POSITIVE_INFINITY),
                    arvm_new_range(&arena,
                                   arvm_new_binary(&arena, ARVM_BINARY_MOD,
                                                   arvm_new_arg_ref(&arena),
                                                   arvm_new_const(&arena, 2)),
                                   1, 1));

  TEST_ASSERT(arvm_is_identical(func.value, exp));
}

void test_short_circuit(void) {
  struct arvm_func func = {
      arvm_new_nary(&arena, ARVM_NARY_AND, 2,
                    arvm_new_call(&arena, NULL, arvm_new_expr(&arena, UNKNOWN)),
                    arvm_new_expr(&arena, UNKNOWN))};
  arvm_optimize_func(&func, &arena, &arena);

  arvm_expr_t exp = arvm_new_call(&arena, NULL, arvm_new_expr(&arena, UNKNOWN));

  TEST_ASSERT(arvm_is_identical(func.value->nary.operands.exprs[1], exp));
}
*/

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unwrap_nary);
  RUN_TEST(test_fold_nary);
  RUN_TEST(test_union_ranges);
  /*RUN_TEST(test_annulment_law);
  RUN_TEST(test_identity_law);
  RUN_TEST(test_idempotent_law);
  RUN_TEST(test_absorption_law);
  RUN_TEST(test_distributive_law);
  RUN_TEST(test_call_inlining);
  RUN_TEST(test_const_step_recursion);
  RUN_TEST(test_short_circuit);*/
  return UNITY_END();
}