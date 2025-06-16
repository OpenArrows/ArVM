#ifndef OPT_H
#define OPT_H

#include "arena.h"
#include "arvm.h"

void arvm_optimize_func(arvm_func_t func, arvm_arena_t *arena,
                        arvm_arena_t *temp_arena);

#endif /* OPT_H */