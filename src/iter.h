#ifndef ITERATOR
#define ITERATOR(inited, init, next, deinit)                                   \
  do {                                                                         \
    if (!(inited)) {                                                           \
      init;                                                                    \
    }                                                                          \
    next;                                                                      \
  iterator_end:                                                                \
    deinit;                                                                    \
    return 0;                                                                  \
  } while (0)

#define YIELD() return 1

#define BREAK() goto iterator_end
#endif