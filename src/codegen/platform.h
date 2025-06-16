#include <stdint.h>

#ifndef ARVM_JIT_PLATFORM
#define ARVM_JIT_PLATFORM

// Detect target OS
#define ARVM_JIT_OS_POSIX 0
#define ARVM_JIT_OS_WIN32 1

#if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) ||              \
    defined(_POSIX_SOURCE)
#define ARVM_JIT_OS ARVM_JIT_OS_POSIX
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||              \
    defined(__TOS_WIN__) || defined(__WINDOWS__)
#define ARVM_JIT_OS ARVM_JIT_OS_WIN32
#endif

// Detect target architecture
#define ARVM_JIT_ARCH_X86 0
#define ARVM_JIT_ARCH_X86_64 1

#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define ARVM_JIT_ARCH ARVM_JIT_ARCH_X86
#elif defined(__amd64) || defined(__amd64__) || defined(_x86_64) ||            \
    defined(_x86_64__)
#define ARVM_JIT_ARCH ARVM_JIT_ARCH_X86_64
#endif

#if defined(ARVM_JIT_OS) && defined(ARVM_JIT_ARCH)
#define ARVM_JIT_SUPPORTED 1
#else
#define ARVM_JIT_SUPPORTED 0
#endif

#endif // ARVM_JIT_PLATFORM