#ifndef ARVM_META_H
#define ARVM_META_H

#include <stddef.h>

#define assert_same_type(a, b) (1 ? (a) : (b))

#define assert_type(value, type) assert_same_type(value, (type){0})

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)assert_same_type(ptr, &(type){0}.member) -                 \
            offsetof(type, member)))

#define item_at(container, type, index, size)                                  \
  ((type *)((char *)assert_type(container, type *) + (index) * (size)))

#endif /* ARVM_META_H */