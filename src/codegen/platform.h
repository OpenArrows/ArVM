#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

// Detect target OS
#define CODEGEN_OS_POSIX 0
#define CODEGEN_OS_WIN32 1

#if defined(unix) || defined(__unix) || defined(_XOPEN_SOURCE) ||              \
    defined(_POSIX_SOURCE)
#define CODEGEN_OS CODEGEN_OS_POSIX
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||              \
    defined(__TOS_WIN__) || defined(__WINDOWS__)
#define CODEGEN_OS CODEGEN_OS_WIN32
#endif

// Detect target architecture
#define CODEGEN_ARCH_I386 0
#define CODEGEN_ARCH_AMD64 1

#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define CODEGEN_ARCH CODEGEN_ARCH_I386
#elif defined(__amd64) || defined(__amd64__) || defined(_x86_64) ||            \
    defined(_x86_64__)
#define CODEGEN_ARCH CODEGEN_ARCH_AMD64
#endif

#if defined(CODEGEN_OS) && defined(CODEGEN_ARCH)
#define CODEGEN_SUPPORTED 1
#else
#define CODEGEN_SUPPORTED 0
#endif

#endif // PLATFORM_H