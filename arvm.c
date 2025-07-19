#include "arvm.h"
#include "bdd.h"
#include "stack.h"
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

  arvm_subdomain_t *domain;

  size_t scc;

  // Used internally to store some additional data alongside functions
  size_t id;
};

typedef ARVM_STACK(arvm_function_t) arvm_func_stack_t;

#define MIN_VAR_MAP_CAPACITY_BITS 4

struct arvm_bdd_var {
  arvm_function_t callee;
  arvm_int_t offset;
};

static inline size_t var_hash(arvm_space_t *space, struct arvm_bdd_var var) {
  const size_t P1 = 12582917;
  return (((size_t)(uintptr_t)var.callee + (size_t)var.offset) * P1) >>
         (sizeof(size_t) * CHAR_BIT - space->var_map.capacity_bits);
}

// Hash map entry that maps vars (function calls) to unique IDs
struct arvm_bdd_var_entry {
  struct arvm_bdd_var var;
  size_t index;
};

static inline arvm_int_t add(arvm_int_t a, arvm_int_t b) {
  return b > ARVM_INFINITY - a ? ARVM_INFINITY : a + b;
}

static inline arvm_int_t sub(arvm_int_t a, arvm_int_t b) {
  return b > a ? 0 : a - b;
}

typedef struct tarjan_vertex {
  size_t index, lowlink;
  bool on_stack;
} tarjan_vertex_t;

static void tarjan_strongconnect(arvm_space_t *space, arvm_function_t func,
                                 tarjan_vertex_t *vert,
                                 arvm_func_stack_t *stack, size_t *index,
                                 size_t *scc_index);

static void tarjan_visit_bdd(arvm_space_t *space, tarjan_vertex_t *v,
                             arvm_bdd_node_t bdd, tarjan_vertex_t *vert,
                             arvm_func_stack_t *stack, size_t *index,
                             size_t *scc_index) {
  if (arvm_bdd_is_leaf(&space->bdd_mgr, bdd))
    return;

  arvm_function_t func = space->vars[arvm_bdd_get_var(bdd)].callee;

  tarjan_vertex_t *w = &vert[func->id];
  if (w->index == 0) {
    tarjan_strongconnect(space, func, vert, stack, index, scc_index);
    if (w->lowlink < v->lowlink)
      v->lowlink = w->lowlink;
  } else if (w->on_stack) {
    if (w->index < v->lowlink)
      v->lowlink = w->index;
  }

  if (v->lowlink == v->index) {
    do {
      func = ARVM_ST_POP(stack);
      w = &vert[func->id];
      w->on_stack = false;
      func->scc = (*scc_index)++;
    } while (w != v);
  }

  tarjan_visit_bdd(space, v, arvm_bdd_get_low(bdd), vert, stack, index,
                   scc_index);
  tarjan_visit_bdd(space, v, arvm_bdd_get_high(bdd), vert, stack, index,
                   scc_index);
}

static void tarjan_strongconnect(arvm_space_t *space, arvm_function_t func,
                                 tarjan_vertex_t *vert,
                                 arvm_func_stack_t *stack, size_t *index,
                                 size_t *scc_index) {
  tarjan_vertex_t *v = &vert[func->id];
  v->index = v->lowlink = (*index)++;

  ARVM_ST_PUSH(stack, func);
  v->on_stack = true;

  arvm_subdomain_t *subdomain = func->domain;
  do {
    tarjan_visit_bdd(space, v, subdomain->value, vert, stack, index, scc_index);
  } while (subdomain->end != ARVM_INFINITY);
}

// Find the SCCs of the call graph. Each SCC will be assigned a unique ID
// https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm#The_algorithm_in_pseudocode
static inline bool tarjan(arvm_space_t *space) {
  bool success = true;

  tarjan_vertex_t *vert = NULL;

  arvm_func_stack_t stack;
  if (!ARVM_ST_INIT(&stack, space->size)) {
    success = false;
    goto tarjan_cleanup;
  }

  if ((vert = calloc(space->size, sizeof(tarjan_vertex_t))) == NULL) {
    success = false;
    goto tarjan_cleanup;
  }

  size_t index = 1, scc_index = 0;
  for (arvm_function_t func = space->tail_function; func != NULL;
       func = func->previous) {
    if (vert[func->id].index == 0)
      tarjan_strongconnect(space, func, vert, &stack, &index, &scc_index);
  }

tarjan_cleanup:
  free(vert);
  ARVM_ST_FREE(&stack);

  return success;
}

void arvm_prepare_space(arvm_space_t *space) {
  { // Assign a unique ID to each function
    size_t id = 0;
    for (arvm_function_t func = space->tail_function; func != NULL;
         func = func->previous) {
      func->id = id++;
      func->scc = 0;
    }
  }

  tarjan(space);
}

arvm_function_t arvm_new_function(arvm_space_t *space) {
  arvm_function_t func = malloc(sizeof(struct arvm_function));
  if (func == NULL)
    return NULL;
  func->space = space;
  func->domain = NULL;
  func->id = 0;
  func->next = NULL;
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

arvm_expr_t arvm_make_true(arvm_space_t *space) {
  return arvm_bdd_one(&space->bdd_mgr);
}

arvm_expr_t arvm_make_false(arvm_space_t *space) {
  return arvm_bdd_zero(&space->bdd_mgr);
}

arvm_expr_t arvm_make_call(arvm_space_t *space, arvm_function_t callee,
                           arvm_int_t offset) {
  struct arvm_bdd_var var = {callee, offset};

  if (space->var_map.capacity_bits > 0) {
    size_t capacity = 1 << space->var_map.capacity_bits;

    size_t i = var_hash(space, var);
    struct arvm_bdd_var_entry entry;
    while ((entry = space->var_map.entries[i++]).var.callee != NULL) {
      if (entry.var.callee == callee && entry.var.offset == offset)
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

arvm_expr_t arvm_not(arvm_expr_t a) { return arvm_bdd_not(a); }

arvm_expr_t arvm_make_ite(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b,
                          arvm_expr_t c) {
  return arvm_bdd_ite(&space->bdd_mgr, a, b, c);
}

arvm_expr_t arvm_make_and(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, b, arvm_make_false(space));
}

arvm_expr_t arvm_make_or(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_not(arvm_make_and(space, arvm_not(a), arvm_not(b)));
}

arvm_expr_t arvm_make_xor(arvm_space_t *space, arvm_expr_t a, arvm_expr_t b) {
  return arvm_make_ite(space, a, arvm_not(b), b);
}

void arvm_set_function_domain(arvm_function_t func, ...) {
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
  if (domain == NULL)
    return;

  size_t i = 0;
  do
    domain[i++] = va_arg(args2, arvm_subdomain_t);
  while (subdomain.end != ARVM_INFINITY);
  va_end(args2);

  free(func->domain);
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

bool arvm_call_function(arvm_function_t func, arvm_int_t t) {
  if (t == 0)
    return false;

  arvm_subdomain_t *subdomain = func->domain;
  while (t > subdomain->end)
    subdomain++;

  arvm_bdd_node_t bdd = subdomain->value;
  while (!arvm_bdd_is_leaf(&func->space->bdd_mgr, bdd)) {
    struct arvm_bdd_var var = func->space->vars[arvm_bdd_get_var(bdd)];
    bdd = arvm_call_function(var.callee, sub(t, var.offset))
              ? arvm_bdd_get_high(bdd)
              : arvm_bdd_get_low(bdd);
  }
  return bdd == arvm_bdd_one(&func->space->bdd_mgr);
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