#include "xalloc.h"
#include "platform.h"
#include <stddef.h>

#if CODEGEN_OS == CODEGEN_OS_POSIX
#include <sys/mman.h>
#include <unistd.h>
#elif CODEGEN_OS == CODEGEN_OS_WIN32
#include <windows.h>
#endif

void *arvm_alloc_x(size_t size) {
  size_t page_size;
#if CODEGEN_OS == CODEGEN_OS_POSIX
  page_size = (size_t)sysconf(_SC_PAGESIZE);
#elif CODEGEN_OS == CODEGEN_OS_WIN32
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  page_size = (size_t)system_info.dwPageSize;
#endif

  size_t alloc_size = (1 + (size - 1) / page_size) * page_size;

#if CODEGEN_OS == CODEGEN_OS_POSIX
  return mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
              MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#elif CODEGEN_OS == CODEGEN_OS_WIN32
  return VirtualAlloc(NULL, alloc_size, MEM_RESERVE | MEM_COMMIT,
                      PAGE_READWRITE);
#else
  unreachable();
#endif
}

void arvm_protect_x(void *ptr, size_t size) {
#if CODEGEN_OS == CODEGEN_OS_POSIX
  mprotect(ptr, size, PROT_READ | PROT_EXEC);
#elif CODEGEN_OS == CODEGEN_OS_WIN32
  DWORD _old_protect;
  VirtualProtect(ptr, size, PAGE_EXECUTE_READ, &_old_protect);
#else
  unreachable();
#endif
}

void arvm_free_x(void *ptr, size_t size) {
#if CODEGEN_OS == CODEGEN_OS_POSIX
  long page_size = sysconf(_SC_PAGESIZE);
  munmap(ptr, (1 + (size - 1) / page_size) * page_size);
#elif CODEGEN_OS == CODEGEN_OS_WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#else
  unreachable();
#endif
}