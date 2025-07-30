#include "arvm.h"
#include "bdd.h"
#include "util/meta.h"
#include "util/pool.h"
#include <assert.h>
#include <limits.h>
#include <omp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct arvm_space {
  arvm_int_t time;

  arvm_pool_t func_pool;
  size_t next_func_id;

  struct {
    arvm_function_t *ptr;
    size_t capacity;
  } func_list;

  arvm_bdd_manager_t bdd_mgr;
};

struct arvm_function {
  union {
    arvm_pool_reserved_t _reserved0;

    struct {
      bool state[2];

      arvm_subdomain_t *domain;
    };
  };

  size_t id;
};

bool arvm_function_initializer(arvm_pool_t *pool, void *func_) {
  arvm_function_t func = func_;
  arvm_space_t space = container_of(pool, struct arvm_space, func_pool);
  func->id = space->next_func_id++;

  if (space->next_func_id > space->func_list.capacity) {
    size_t new_capacity =
        space->func_list.capacity == 0 ? 1 : space->func_list.capacity * 2;
    while (space->next_func_id > new_capacity)
      new_capacity *= 2;
    if (new_capacity < space->func_list.capacity)
      return false; // integer overflow

    arvm_function_t *new_func_list =
        realloc(space->func_list.ptr, sizeof(arvm_function_t *) * new_capacity);
    if (new_func_list == NULL)
      return false;

    memset(&new_func_list[space->func_list.capacity], 0,
           sizeof(arvm_function_t *) *
               (new_capacity - space->func_list.capacity));

    space->func_list.ptr = new_func_list;
    space->func_list.capacity = new_capacity;
  }

  return true;
}

arvm_space_t arvm_create_space() {
  arvm_space_t space = malloc(sizeof(struct arvm_space));
  if (space == NULL)
    return NULL;

  space->time = 0;
  space->func_pool = ARVM_POOL(sizeof(struct arvm_function),
                               ARVM_FUNC_BLOCK_SIZE, arvm_function_initializer);
  space->next_func_id = 0;
  space->func_list.ptr = NULL;
  space->func_list.capacity = 0;
  space->bdd_mgr = ARVM_BDD_MANAGER();
  return space;
}

void arvm_dispose_space(arvm_space_t space) {
  arvm_pool_clear(&space->func_pool);
  arvm_bdd_free(&space->bdd_mgr);
}

void arvm_update(arvm_space_t space, arvm_int_t dt) {
  for (arvm_int_t i = 0; i < dt; i++, space->time++) {
    int prev = space->time % 2, next = (space->time + 1) % 2;

#pragma omp parallel for
    for (size_t j = 0; j < space->func_list.capacity; j++) {
      arvm_function_t func = space->func_list.ptr[j];
      if (func == NULL)
        continue;

      arvm_subdomain_t *subdomain = func->domain;
      while (space->time > subdomain->end)
        subdomain++;

      arvm_bdd_node_t bdd = subdomain->value;
      while (!ARVM_BDD_IS_LEAF(bdd)) {
        arvm_function_t callee = space->func_list.ptr[ARVM_BDD_VAR(bdd)];
        bdd = callee->state[prev] ? ARVM_BDD_HI(bdd) : ARVM_BDD_LO(bdd);
      }

      func->state[next] = bdd == ARVM_BDD_TRUE;
    }
  }
}

bool arvm_reserve(arvm_space_t space, size_t size) {
  return arvm_pool_reserve(&space->func_pool, size);
}

arvm_function_t arvm_new_function(arvm_space_t space) {
  arvm_function_t func = arvm_pool_alloc(&space->func_pool);
  if (func == NULL)
    return NULL;
  space->func_list.ptr[func->id] = func;
  func->domain = NULL;
  func->state[0] = func->state[1] = false;
  return func;
}

arvm_expr_t arvm_true() { return ARVM_BDD_TRUE; }

arvm_expr_t arvm_false() { return ARVM_BDD_FALSE; }

arvm_expr_t arvm_make_call(arvm_space_t space, arvm_function_t callee) {
  return arvm_bdd_var(&space->bdd_mgr, callee->id);
}

arvm_expr_t arvm_not(arvm_expr_t a) { return ARVM_BDD_NOT(a); }

arvm_expr_t arvm_make_ite(arvm_space_t space, arvm_expr_t a, arvm_expr_t b,
                          arvm_expr_t c) {
  return arvm_bdd_ite(&space->bdd_mgr, a, b, c);
}

arvm_expr_t arvm_make_and(arvm_space_t space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, b, arvm_false());
}

arvm_expr_t arvm_make_or(arvm_space_t space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_not(arvm_make_and(space, arvm_not(a), arvm_not(b)));
}

arvm_expr_t arvm_make_xor(arvm_space_t space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, arvm_not(b), b);
}

static inline void free_domain(arvm_space_t space, arvm_function_t func) {
  if (func->domain == NULL)
    return;

  arvm_subdomain_t *subdomain = func->domain;
  do
    arvm_bdd_free_node(&space->bdd_mgr, subdomain->value);
  while ((subdomain++)->end != ARVM_INFINITY);
  free(func->domain);
}

bool arvm_set_function_domain(arvm_space_t space, arvm_function_t func,
                              arvm_subdomain_t *domain) {
  free_domain(space, func);

  func->state[0] = func->state[1] = false;

  size_t subdomain_count = 0;

  arvm_subdomain_t *subdomain = domain;
  do
    subdomain_count++;
  while ((subdomain++)->end != ARVM_INFINITY);

  arvm_subdomain_t *new_domain =
      malloc(sizeof(arvm_subdomain_t) * subdomain_count);
  if (new_domain == NULL)
    return false;

  size_t i = 0;
  subdomain = domain;
  do
    new_domain[i++] = *subdomain;
  while ((subdomain++)->end != ARVM_INFINITY);

  func->domain = new_domain;
  return true;
}

void arvm_delete_function(arvm_space_t space, arvm_function_t func) {
  free_domain(space, func);
  space->func_list.ptr[func->id] = NULL;
  arvm_pool_free(&space->func_pool, func);
}

bool arvm_get_function_value(arvm_space_t space, arvm_function_t func) {
  return func->state[space->time % 2];
}