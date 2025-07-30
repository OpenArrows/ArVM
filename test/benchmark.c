// This benchmark is designed to imitate a Logic Arrows map

#include <arvm.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FUNC_COUNT 5000
#define NUM_TICKS 100000

#define NUM_ITERATIONS 16

#define FUNC_DEPS 4

#define SEED 0xCABA1ABA

int main(void) {
  arvm_space_t space = arvm_create_space();

  arvm_function_t funcs[FUNC_COUNT];
  arvm_reserve(space, FUNC_COUNT);
  for (size_t i = 0; i < FUNC_COUNT; i++)
    funcs[i] = arvm_new_function(space);

  srand(SEED);

  clock_t total = 0;

  for (size_t i = 0; i < NUM_ITERATIONS; i++) {
    for (size_t i = 0; i < FUNC_COUNT; i++) {
      arvm_function_t func = funcs[i];
      arvm_function_t deps[FUNC_DEPS];
      for (size_t j = 0; j < FUNC_DEPS; j++)
        deps[j] = funcs[(i + j + 1) % FUNC_COUNT];
      switch (rand() % 4) {
      case 0: // source block
        arvm_set_function_domain(
            space, func, (arvm_subdomain_t[]){{ARVM_INFINITY, arvm_true()}});
        break;
      case 1: // pulse block
        arvm_set_function_domain(
            space, func,
            (arvm_subdomain_t[]){{1, arvm_true()},
                                 {ARVM_INFINITY, arvm_false()}});
        break;
      case 2: // or
      {
        arvm_expr_t value = arvm_false();
        for (size_t j = 0; j < FUNC_DEPS; j++)
          value = arvm_make_or(space, value, arvm_make_call(space, deps[j]));
        arvm_set_function_domain(space, func,
                                 (arvm_subdomain_t[]){{ARVM_INFINITY, value}});
        break;
      }
      case 3: // t flip-flop
      {
        arvm_expr_t value = arvm_make_call(space, func);
        for (size_t j = 0; j < FUNC_DEPS; j++)
          value = arvm_make_xor(space, value, arvm_make_call(space, deps[j]));
        arvm_set_function_domain(space, func,
                                 (arvm_subdomain_t[]){{ARVM_INFINITY, value}});
        break;
      }
      }
    }

    clock_t start = clock();
    arvm_update(space, NUM_TICKS);
    total += clock() - start;
  }

  printf("total: %fs\n%fs per iteration (%llu ticks) / %fmus per tick\n",
         (float)total / CLOCKS_PER_SEC,
         (float)(total) / NUM_ITERATIONS / CLOCKS_PER_SEC,
         (unsigned long long)NUM_TICKS,
         (float)(total) * 1000000 / NUM_TICKS / NUM_ITERATIONS /
             CLOCKS_PER_SEC);

  arvm_dispose_space(space);
  return 0;
}