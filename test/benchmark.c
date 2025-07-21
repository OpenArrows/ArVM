#include <arvm.h>
#include <locale.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#define NUM_ITERATIONS 10
#define NUM_TICKS 10000000

arvm_space_t space = {};

int main(void) {
  setlocale(LC_ALL, "en_US.utf8");
  fwide(stdout, 1);

  arvm_function_t f = arvm_new_function(&space), g = arvm_new_function(&space),
                  h = arvm_new_function(&space);

  arvm_set_function_domain(h, (arvm_subdomain_t[]){
                                  {ARVM_INFINITY, arvm_true()},
                              });

  arvm_set_function_domain(
      g, (arvm_subdomain_t[]){
             {ARVM_INFINITY, arvm_make_xor(&space, arvm_make_call(&space, f),
                                           arvm_make_call(&space, h))},
         });

  arvm_set_function_domain(
      f, (arvm_subdomain_t[]){
             {ARVM_INFINITY, arvm_make_xor(&space, arvm_make_call(&space, g),
                                           arvm_make_call(&space, h))},
         });

  clock_t total = 0;

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    clock_t start = clock();
    arvm_update(&space, NUM_TICKS);
    total += clock() - start;
  }
  wprintf(L"total: %fs\n%fs per iteration (%llu ticks) / %fmus per tick\n",
          (float)total / CLOCKS_PER_SEC,
          (float)(total) / NUM_ITERATIONS / CLOCKS_PER_SEC,
          (unsigned long long)NUM_TICKS,
          (float)(total) * 1000000 / NUM_TICKS / NUM_ITERATIONS /
              CLOCKS_PER_SEC);

  arvm_dispose_space(&space);
  return 0;
}