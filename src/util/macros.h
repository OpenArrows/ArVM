#ifndef MACROS_H
#define MACROS_H

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define UNIQUE(x) CONCAT(x, __LINE__)

#define EXPAND(...) __VA_ARGS__

#define UNWRAP(x) EXPAND x

#define COMMA(x) x,

#define lengthof(x) (sizeof((x)) / sizeof(*(x)))

#endif /* MACROS_H */