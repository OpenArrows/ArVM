#include "arvm.h"
#include "bdd.h"
#include "stack.h"
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// All unary boolean ArVM IR functions are piecewise defined. The single
// integer argument t is the current simulation time (tick)
struct arvm_function {
  arvm_space_t *space;

  struct {
    arvm_function_t previous;
    arvm_function_t next;
  };

  struct {
    bool curr, prev;
    arvm_int_t time;
  } state;

  arvm_subdomain_t *domain;
};

typedef ARVM_STACK(arvm_function_t) arvm_func_stack_t;

#define MIN_VAR_MAP_CAPACITY_BITS 4

struct arvm_bdd_var {
  arvm_function_t callee;
};

static inline size_t var_hash(arvm_space_t *space, struct arvm_bdd_var var) {
  const size_t P1 = 12582917;
  return ((size_t)(uintptr_t)var.callee * P1) >>
         (sizeof(size_t) * CHAR_BIT - space->var_map.capacity_bits);
}

// Hash map entry that maps vars (function calls) to unique IDs
struct arvm_bdd_var_entry {
  struct arvm_bdd_var var;
  size_t index;
};

void arvm_update(arvm_space_t *space, arvm_int_t dt) {
  for (arvm_int_t i = 0; i < dt; i++) {
    for (arvm_function_t func = space->tail_function; func != NULL;
         func = func->previous) {
      func->state.prev = func->state.curr;
      func->state.time++;

      arvm_subdomain_t *subdomain = func->domain;
      while (func->state.time > subdomain->end)
        subdomain++;

      arvm_bdd_node_t bdd = subdomain->value;
      while (!ARVM_BDD_IS_LEAF(&space->bdd_mgr, bdd)) {
        struct arvm_bdd_var var = space->vars[ARVM_BDD_VAR(bdd)];
        assert(var.callee->state.time == func->state.time ||
               var.callee->state.time == func->state.time - 1);
        bdd = (var.callee->state.time == func->state.time
                   ? var.callee->state.prev
                   : var.callee->state.curr)
                  ? ARVM_BDD_HI(bdd)
                  : ARVM_BDD_LO(bdd);
      }

      func->state.curr = bdd == ARVM_BDD_TRUE;
    }
  }
}

void arvm_dispose_space(arvm_space_t *space) {
  arvm_function_t func = space->tail_function;
  space->tail_function = NULL;
  while (func != NULL) {
    arvm_function_t next = func->previous;
    arvm_delete_function(func);
    func = next;
  }
  space->size = 0;

  arvm_bdd_free(&space->bdd_mgr);

  free(space->var_map.entries);
  space->var_map.entries = NULL;
  space->var_map.length = 0;
  space->var_map.capacity_bits = 0;

  free(space->vars);
  space->vars = NULL;

  space->var_index = 0;
}

arvm_function_t arvm_new_function(arvm_space_t *space) {
  arvm_function_t func = calloc(1, sizeof(struct arvm_function));
  if (func == NULL)
    return NULL;
  func->space = space;
  func->previous = space->tail_function;
  if (space->tail_function != NULL)
    space->tail_function->next = func;
  space->tail_function = func;
  space->size++;
  return func;
}

// Inserts a new var entry into the map
static inline void put_var_entry(arvm_space_t *space,
                                 struct arvm_bdd_var_entry entry) {
  size_t capacity = 1 << space->var_map.capacity_bits;

  size_t i = var_hash(space, entry.var);
  while (space->var_map.entries[i].var.callee != NULL) {
    if (++i >= capacity)
      i = 0;
  }

  space->var_map.length++;
  space->var_map.entries[i] = entry;
}

arvm_expr_t arvm_true() { return ARVM_BDD_TRUE; }

arvm_expr_t arvm_false() { return ARVM_BDD_FALSE; }

arvm_expr_t arvm_make_call(arvm_space_t *space, arvm_function_t callee) {
  struct arvm_bdd_var var = {callee};

  if (space->var_map.capacity_bits > 0) {
    size_t capacity = 1 << space->var_map.capacity_bits;

    size_t i = var_hash(space, var);
    struct arvm_bdd_var_entry entry;
    while ((entry = space->var_map.entries[i++]).var.callee != NULL) {
      if (entry.var.callee == callee)
        return arvm_bdd_var(&space->bdd_mgr, entry.index);

      if (i >= capacity)
        i = 0;
    }

    if (space->var_map.length >= capacity) {
      // Grow the set

      if (space->var_map.capacity_bits + 1 > sizeof(size_t) * CHAR_BIT)
        return NULL; // overflow
      size_t new_capacity = 1 << (space->var_map.capacity_bits + 1);

      struct arvm_bdd_var *new_vars =
          realloc(space->vars, sizeof(struct arvm_bdd_var) * new_capacity);
      if (new_vars == NULL)
        return NULL;
      space->vars = new_vars;

      struct arvm_bdd_var_entry *new_entries =
          calloc(new_capacity, sizeof(struct arvm_bdd_var_entry));
      if (new_entries == NULL)
        return NULL;

      struct arvm_bdd_var_entry *old_entries = space->var_map.entries;
      space->var_map.entries = new_entries;
      space->var_map.length = 0;
      space->var_map.capacity_bits++;

      for (size_t i = 0; i < new_capacity; i++) {
        struct arvm_bdd_var_entry node = old_entries[i];
        if (node.var.callee != NULL)
          put_var_entry(space, node);
      }

      free(old_entries);
    }
  } else {
    space->vars =
        malloc(sizeof(struct arvm_bdd_var) * (1 << MIN_VAR_MAP_CAPACITY_BITS));
    if (space->vars == NULL)
      return NULL;
    space->var_map.entries = calloc(1 << MIN_VAR_MAP_CAPACITY_BITS,
                                    sizeof(struct arvm_bdd_var_entry));
    if (space->var_map.entries == NULL)
      return NULL;
    space->var_map.capacity_bits = MIN_VAR_MAP_CAPACITY_BITS;
  }

  put_var_entry(space, (struct arvm_bdd_var_entry){var, space->var_index});
  space->vars[space->var_index] = var;
  return arvm_bdd_var(&space->bdd_mgr, space->var_index++);
}

arvm_expr_t arvm_not(arvm_expr_t a) { return ARVM_BDD_NOT(a); }

arvm_expr_t arvm_make_ite(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b,
                          arvm_expr_t c) {
  return arvm_bdd_ite(&space->bdd_mgr, a, b, c);
}

arvm_expr_t arvm_make_and(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, b, arvm_false());
}

arvm_expr_t arvm_make_or(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_not(arvm_make_and(space, arvm_not(a), arvm_not(b)));
}

arvm_expr_t arvm_make_xor(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, arvm_not(b), b);
}

void arvm_set_function_domain(arvm_function_t func, ...) {
  free(func->domain);

  va_list args, args2;
  va_start(args, func);
  va_copy(args2, args);

  size_t subdomain_count = 0;

  arvm_subdomain_t subdomain;
  do {
    subdomain = va_arg(args, arvm_subdomain_t);
    subdomain_count++;
  } while (subdomain.end != ARVM_INFINITY);
  va_end(args);

  arvm_subdomain_t *domain = malloc(sizeof(arvm_subdomain_t) * subdomain_count);
  if (domain == NULL) {
    func->domain = NULL;
    return;
  }

  size_t i = 0;
  do
    domain[i++] = subdomain = va_arg(args2, arvm_subdomain_t);
  while (subdomain.end != ARVM_INFINITY);
  va_end(args2);

  func->domain = domain;
}

void arvm_delete_function(arvm_function_t func) {
  if (func->next)
    func->next->previous = func->previous;
  if (func->previous)
    func->previous->next = func->next;

  func->space->size--;

  free(func->domain);
  free(func);
}

bool arvm_get_function_value(arvm_function_t func) { return func->state.curr; }