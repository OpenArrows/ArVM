#include "arvm.h"
#include "math.h"
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline arvm_int_t add(arvm_int_t a, arvm_int_t b) {
  return b > ARVM_INFINITY - a ? ARVM_INFINITY : a + b;
}

static inline arvm_int_t sub(arvm_int_t a, arvm_int_t b) {
  return b > a ? 0 : a - b;
}

static inline arvm_int_t pow2(arvm_int_t x) { return 1 << x; }

static inline arvm_int_t ones(arvm_int_t x) { return pow2(x) - 1; }

static inline void optimize_func(arvm_function_t func) {
  // TODO
}

void arvm_optimize_space(arvm_space_t *space) {
  struct stack {
    struct stack_entry {
      arvm_function_t func;
      arvm_subdomain_t *subdomain;
      size_t callee_index;
    } *data;
    size_t offset;
  } stack = {malloc(sizeof(*stack.data) * space->size), 0};
  if (stack.data == NULL)
    return;
  memset(stack.data, 0, sizeof(*stack.data) * space->size);

  // Initialize
  for (arvm_function_t func = space->tail_function; func != NULL;
       func = func->previous)
    func->state = ARVM_TODO;

  for (arvm_function_t func = space->tail_function; func != NULL;
       func = func->previous) {
    if (func->state != ARVM_TODO)
      continue;

    func->state = ARVM_VISITING;
    stack.data[stack.offset++] = (struct stack_entry){func, func->domain, 0};

    while (stack.offset > 0) {
      struct stack_entry *entry = &stack.data[stack.offset - 1];
      /*if (entry->callee_index < entry->subdomain->operand_count) {
        arvm_function_t callee =
            entry->subdomain->operands[entry->callee_index++].func;
        if (callee->state != ARVM_TODO)
          continue;
        callee->state = ARVM_VISITING;
        stack.data[stack.offset++] =
            (struct stack_entry){callee, callee->domain, 0};
      } else if (entry->subdomain->end < ARVM_INFINITY) {
        entry->subdomain++;
      } else {
        stack.offset--;
        optimize_func(entry->func);
        entry->func->state = ARVM_VISITED;
      }*/
    }
  }

  free(stack.data);
}

arvm_function_t arvm_new_function(arvm_space_t *space) {
  arvm_function_t func = malloc(sizeof(struct arvm_function));
  if (func == NULL)
    return NULL;
  func->space = space;
  func->counterpart = NULL;
  func->domain = NULL;
  func->next = NULL;
  func->previous = space->tail_function;
  if (space->tail_function != NULL)
    space->tail_function->next = func;
  space->tail_function = func;
  space->size++;
  return func;
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

bool arvm_call_function(arvm_function_t func, arvm_int_t arg) {
  if (arg == 0)
    return false;

  arvm_subdomain_t *subdomain = func->domain;
  while (arg > subdomain->end)
    subdomain++;

  arvm_bdd_node_t bdd = subdomain->bdd;
  while (!arvm_is_const(bdd))
    bdd = get_var_val(bdd->var) ? bdd->hi : bdd->lo;
  return bdd == arvm_one();
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
}