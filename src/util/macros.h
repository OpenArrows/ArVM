#ifndef MACROS_H
#define MACROS_H

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define UNIQUE(x) CONCAT(x, __LINE__)

#define UNWRAP_(...) __VA_ARGS__
#define UNWRAP(x) UNWRAP_ x

#define lengthof(x) (sizeof((x)) / sizeof(*(x)))

#endif /* MACROS_H */