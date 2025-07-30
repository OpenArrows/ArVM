#include <arvm.h>
#include <unity.h>

arvm_space_t space;

void setUp(void) { space = arvm_create_space(); }

void tearDown(void) { arvm_dispose_space(space); }

void test(void) {
  arvm_function_t f = arvm_new_function(space), g = arvm_new_function(space),
                  h = arvm_new_function(space);

  arvm_set_function_domain(space, h,
                           (arvm_subdomain_t[]){
                               {ARVM_INFINITY, arvm_true()},
                           });

  arvm_set_function_domain(
      space, g,
      (arvm_subdomain_t[]){
          {ARVM_INFINITY, arvm_make_xor(space, arvm_make_call(space, f),
                                        arvm_make_call(space, h))},
      });

  arvm_set_function_domain(
      space, f,
      (arvm_subdomain_t[]){
          {ARVM_INFINITY, arvm_make_xor(space, arvm_make_call(space, g),
                                        arvm_make_call(space, h))},
      });

  TEST_ASSERT_FALSE(arvm_get_function_value(space, g));

  arvm_update(space, 1);
  TEST_ASSERT_FALSE(arvm_get_function_value(space, g));

  arvm_update(space, 1);
  TEST_ASSERT_TRUE(arvm_get_function_value(space, g));

  arvm_update(space, 1);
  TEST_ASSERT_FALSE(arvm_get_function_value(space, g));

  arvm_update(space, 1);
  TEST_ASSERT_TRUE(arvm_get_function_value(space, g));

  arvm_update(space, 1);
  TEST_ASSERT_FALSE(arvm_get_function_value(space, g));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test);
  return UNITY_END();
}