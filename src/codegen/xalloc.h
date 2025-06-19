#ifndef XALLOC_H
#define XALLOC_H

#include <stddef.h>

void *arvm_alloc_x(size_t size);

void arvm_protect_x(void *ptr, size_t size);

void arvm_free_x(void *ptr, size_t size);

#endif /* XALLOC_H */