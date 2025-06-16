#include <arvm.h>
#include <codegen/asm.h>
#include <unity.h>

xbuf_t buf;
asm_t asm_;

void setUp(void) {
  buf = (xbuf_t){};
  asm_ = (asm_t){&buf};
}

void tearDown(void) { asm_free(&asm_); }

void test_asm_arg(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(123, func(123));
}

void test_asm_const(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_const(&asm_, 1);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(1, func(123));
}

void test_asm_mod(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_mod(&asm_, 4);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(3, func(123));
}

void test_asm_add(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_const(&asm_, -1);
  asm_add(&asm_);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(122, func(123));
}

void test_asm_or(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_const(&asm_, 0);
  asm_or(&asm_);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(1, func(1));
}

void test_asm_and(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_const(&asm_, 1);
  asm_and(&asm_);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(0, func(0));
}

void test_asm_xor(void) {
  asm_label_t func_lbl = {};

  asm_label(&asm_, &func_lbl);
  asm_arg(&asm_);
  asm_const(&asm_, 1);
  asm_xor(&asm_);
  asm_ret(&asm_);

  asm_build(&asm_);

  arvm_val_t (*func)(arvm_val_t) = asm_ptr(&asm_, func_lbl);

  TEST_ASSERT_EQUAL(0, func(1));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_asm_arg);
  RUN_TEST(test_asm_const);
  RUN_TEST(test_asm_mod);
  RUN_TEST(test_asm_add);
  RUN_TEST(test_asm_or);
  RUN_TEST(test_asm_and);
  RUN_TEST(test_asm_xor);
  return UNITY_END();
}
