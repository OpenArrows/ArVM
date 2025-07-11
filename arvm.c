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

static int cmp_caller_count(const void *a, const void *b) {
  size_t a_caller_count = (*(const arvm_function_t *)a)->caller_count;
  size_t b_caller_count = (*(const arvm_function_t *)b)->caller_count;
  return (a_caller_count > b_caller_count) - (a_caller_count < b_caller_count);
}

// Inline functions calls.
// The main part of the algorithm consists of dividing the function domain
// into subdomains, operands of which are the operands of each callee on the
// given intervals.
static inline void inline_calls(arvm_function_t func) {
  arvm_subdomain_t *opt_domain = NULL;
  size_t opt_subdomain_idx = 0;

  arvm_subdomain_t *subdomain = func->domain;
  arvm_int_t subdomain_start = 0; // not inclusive
  for (;;) {
    size_t total_operands = 0;

    // Find the minimal interval where all callee subdomains are consistent
    arvm_subdomain_t **callee_subdomains =
        malloc(sizeof(arvm_subdomain_t *) * subdomain->operand_count);
    if (callee_subdomains == NULL)
      goto cleanup;

    arvm_int_t subdomain_end = subdomain->end;
    for (size_t i = 0; i < subdomain->operand_count; i++) {
      arvm_operand_t operand = subdomain->operands[i];
      arvm_function_t callee = operand.func;
      // Do not inline functions that are not optimized
      if (callee->state != ARVM_VISITED) {
        total_operands++;
        continue;
      }

      arvm_subdomain_t *callee_subdomain =
          NULL; // all functions have an implicit subdomain (0; 0] that is
                // always false
      arvm_int_t callee_abs_end = operand.offset;
      while (callee_abs_end <= subdomain_start) {
        if (callee_subdomain == NULL)
          callee_subdomain = callee->domain;
        else
          callee_subdomain++;
        callee_abs_end = add(callee_subdomain->end, operand.offset);
      }
      if (callee_abs_end < subdomain_end)
        subdomain_end = callee_abs_end;

      callee_subdomains[i] = callee_subdomain;

      if (callee_subdomain != NULL)
        total_operands += callee_subdomain->operand_count;
    }

    if (total_operands > sizeof(uintmax_t) * CHAR_BIT)
      goto cleanup;

    arvm_subdomain_t *new_domain =
        realloc(opt_domain, sizeof(arvm_subdomain_t) * (opt_subdomain_idx + 1));
    if (new_domain == NULL)
      goto cleanup;
    opt_domain = new_domain;

    arvm_subdomain_t *new_subdomain = &opt_domain[opt_subdomain_idx++];
    new_subdomain->end = subdomain_end;
    new_subdomain->operand_count = total_operands;
    new_subdomain->operands = malloc(sizeof(arvm_operand_t) * total_operands);
    size_t table_len = pow2(total_operands);
    new_subdomain->table = malloc(sizeof(arvm_operand_t) * table_len);
    if (new_subdomain->operands == NULL || new_subdomain->table == NULL)
      goto cleanup;

    size_t op_i = 0;
    for (size_t i = 0; i < subdomain->operand_count; i++) {
      arvm_operand_t operand = subdomain->operands[i];
      if (operand.func->state != ARVM_VISITED) {
        new_subdomain->operands[op_i++] = operand;
        continue;
      }

      arvm_subdomain_t *callee_subdomain = callee_subdomains[i];
      if (callee_subdomain == NULL)
        continue;
      for (size_t j = 0; j < callee_subdomain->operand_count; j++) {
        arvm_operand_t sub_operand = callee_subdomain->operands[j];
        sub_operand.offset += subdomain->operands[i].offset;
        new_subdomain->operands[op_i++] = sub_operand;
      }
    }

    for (size_t i = 0; i < table_len; i++) {
      size_t idx = 0;

      size_t offset = 0;
      for (size_t j = 0; j < subdomain->operand_count; j++) {
        arvm_subdomain_t *callee_subdomain = callee_subdomains[j];
        idx <<= 1;
        if (subdomain->operands[j].func->state != ARVM_VISITED) {
          offset++;
          idx |= (i >> (total_operands - offset)) & 1;
        } else if (callee_subdomain != NULL) {
          size_t op_count = callee_subdomain->operand_count;
          offset += op_count;
          size_t callee_idx =
              (i >> (total_operands - offset)) & ones(op_count);
          idx |= callee_subdomain->table[callee_idx];
        }
      }

      new_subdomain->table[i] = subdomain->table[idx];
    }

    subdomain_start = subdomain_end;
    if (subdomain_end == subdomain->end) {
      if (subdomain->end == ARVM_INFINITY)
        break;
      else
        subdomain++;
    }

    continue;

  cleanup:
    for (size_t i = 0; i < opt_subdomain_idx; i++) {
      arvm_subdomain_t subdomain = opt_domain[i];
      free(subdomain.operands);
      free(subdomain.table);
    }
    free(callee_subdomains);
    free(opt_domain);
    opt_domain = NULL;
    break;
  }

  if (opt_domain != NULL)
    func->domain = opt_domain;
}

static inline void optimize_func(arvm_function_t func) { inline_calls(func); }

void arvm_optimize_space(arvm_space_t *space) {
  arvm_function_t *functions = malloc(sizeof(arvm_function_t) * space->size);
  if (functions == NULL)
    return;

  struct stack {
    struct stack_entry {
      arvm_function_t func;
      arvm_subdomain_t *subdomain;
      size_t callee_index;
    } *data;
    size_t offset;
  } stack = {malloc(sizeof(*stack.data) * space->size), 0};
  if (stack.data == NULL) {
    free(functions);
    return;
  }

  // Initialize
  size_t i = 0;
  for (arvm_function_t func = space->tail_function; func != NULL;
       func = func->previous) {
    func->caller_count = 0;
    func->state = ARVM_TODO;
    functions[i++] = func;
  }

  // Count callers
  for (arvm_function_t func = space->tail_function; func != NULL;
       func = func->previous) {
    arvm_subdomain_t *subdomain = func->domain;
    do {
      for (size_t i = 0; i < subdomain->operand_count; i++) {
        arvm_function_t callee = subdomain->operands[i].func;
        callee->callee_processed = false;
      }
    } while ((subdomain++)->end != ARVM_INFINITY);

    subdomain = func->domain;
    do {
      for (size_t i = 0; i < subdomain->operand_count; i++) {
        arvm_function_t callee = subdomain->operands[i].func;
        if (!callee->callee_processed) {
          callee->caller_count++;
          callee->callee_processed = true;
        }
      }
    } while ((subdomain++)->end != ARVM_INFINITY);
  }

  qsort(functions, space->size, sizeof(arvm_function_t), cmp_caller_count);

  memset(stack.data, 0, sizeof(*stack.data) * space->size);

  for (size_t i = 0; i < space->size; i++) {
    if (functions[i]->state != ARVM_TODO)
      continue;

    functions[i]->state = ARVM_VISITING;
    stack.data[stack.offset++] =
        (struct stack_entry){functions[i], functions[i]->domain, 0};

    while (stack.offset > 0) {
      struct stack_entry *entry = &stack.data[stack.offset - 1];
      if (entry->callee_index < entry->subdomain->operand_count) {
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
      }
    }
  }

  free(stack.data);
  free(functions);
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
  do {
    domain[i] = subdomain = va_arg(args2, arvm_subdomain_t);
    domain[i].operands = NULL;
    domain[i].table = NULL;
    size_t operands_size = sizeof(arvm_operand_t) * subdomain.operand_count;
    domain[i].operands = malloc(operands_size);
    if (domain[i].operands == NULL)
      goto cleanup;
    memcpy(domain[i].operands, subdomain.operands, operands_size);
    size_t table_size = sizeof(bool) * pow2(subdomain.operand_count);
    domain[i].table = malloc(table_size);
    if (domain[i].table == NULL)
      goto cleanup;
    memcpy(domain[i].table, subdomain.table, table_size);
    continue;

  cleanup:
    do {
      free(domain[i].operands);
      free(domain[i].table);
    } while (i-- > 0);
    free(domain);
    return;
  } while (subdomain.end != ARVM_INFINITY);
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

  arvm_subdomain_t *subdomain = func->domain;
  do {
    free(subdomain->operands);
    free(subdomain->table);
  } while ((subdomain++)->end != ARVM_INFINITY);

  free(func->domain);
  free(func);
}

bool arvm_call_function(arvm_function_t func, arvm_int_t arg) {
  if (arg == 0)
    return false;

  arvm_subdomain_t *subdomain = func->domain;
  while (arg > subdomain->end)
    subdomain++;

  size_t idx = 0;
  for (size_t i = 0; i < subdomain->operand_count; i++) {
    arvm_operand_t operand = subdomain->operands[i];
    bool operand_value =
        arvm_call_function(operand.func, sub(arg, operand.offset));
    idx <<= 1;
    idx |= operand_value;
  }
  return subdomain->table[idx];
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