#include <arvm.h>
#include <unity.h>

arvm_space_t space = {};

void setUp(void) {}

void tearDown(void) { arvm_dispose_space(&space); }

void test(void) {
  arvm_function_t f = arvm_new_function(&space), g = arvm_new_function(&space),
                  h = arvm_new_function(&space);

  arvm_set_function_domain(h, (arvm_subdomain_t[]){
                                  {ARVM_INFINITY, arvm_one()},
                              });

  arvm_set_function_domain(
      g, (arvm_subdomain_t[]){
             {ARVM_INFINITY, arvm_xor(arvm_var(1), arvm_var(3))},
         });

  arvm_set_function_domain(
      f, (arvm_subdomain_t[]){
             {ARVM_INFINITY, arvm_xor(arvm_var(2), arvm_var(3))},
         });

  arvm_optimize_space(&space);

  TEST_ASSERT_FALSE(arvm_call_function(g, 0));
  TEST_ASSERT_FALSE(arvm_call_function(g, 1));
  TEST_ASSERT_TRUE(arvm_call_function(g, 2));
  TEST_ASSERT_FALSE(arvm_call_function(g, 3));
  TEST_ASSERT_TRUE(arvm_call_function(g, 4));
  TEST_ASSERT_FALSE(arvm_call_function(g, 5));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test);
  return UNITY_END();
}