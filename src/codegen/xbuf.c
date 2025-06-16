#include "xbuf.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>

#if ARVM_JIT_OS == ARVM_JIT_OS_POSIX
#include <sys/mman.h>
#include <unistd.h>
#elif ARVM_JIT_OS == ARVM_JIT_OS_WIN32
#include <windows.h>
#endif

void *xbuf_write(xbuf_t *buf, void *data, size_t length) {
  char *data_buf = data;

  xbuf_block_t *block;
  if (buf->tail == NULL) {
    block = buf->tail = malloc(sizeof(xbuf_block_t));
    block->header.prev = NULL;
    block->header.offset = 0;
  } else {
    block = buf->tail;
  }

  void *result = &block->data[block->header.offset];
  buf->offset += length;

  while (length > 0) {
    size_t size = length;
    if (block->header.offset + size > sizeof(block->data))
      size = sizeof(block->data) - block->header.offset;
    memcpy(&block->data[block->header.offset], data_buf, size);
    block->header.offset += size;
    length -= size;
    data_buf += size;

    if (block->header.offset == sizeof(block->data)) {
      xbuf_block_t *new_block = buf->tail = malloc(sizeof(xbuf_block_t));
      new_block->header.prev = block;
      new_block->header.offset = 0;
      block = new_block;
    }
  }

  return result;
}

void xbuf_free(xbuf_t *buf) {
  xbuf_block_t *block = buf->tail;
  buf->tail = NULL;
  while (block != NULL) {
    xbuf_block_t *next_block = block->header.prev;
    free(block);
    block = next_block;
  }
}

void xbuf_map(xbuf_t *buf) {
  size_t page_size;
#if ARVM_JIT_OS == ARVM_JIT_OS_POSIX
  page_size = (size_t)sysconf(_SC_PAGESIZE);
#elif ARVM_JIT_OS == ARVM_JIT_OS_WIN32
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  page_size = (size_t)system_info.dwPageSize;
#endif

  size_t size = 1 + ((buf->offset - 1) / page_size);

#if ARVM_JIT_OS == ARVM_JIT_OS_POSIX
  buf->xptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#elif ARVM_JIT_OS == ARVM_JIT_OS_WIN32
  buf->xptr =
      VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  unreachable();
#endif

  char *data = buf->xptr;

  xbuf_block_t *block = buf->tail;
  while (block != NULL) {
    memcpy(data, block->data, block->header.offset);
    data += block->header.offset;
    block = block->header.prev;
  }

#if ARVM_JIT_OS == ARVM_JIT_OS_POSIX
  mprotect(buf->xptr, buf->offset, PROT_READ | PROT_EXEC);
#elif ARVM_JIT_OS == ARVM_JIT_OS_WIN32
  DWORD _old_protect;
  VirtualProtect(buf->xptr, buf->offset, PAGE_EXECUTE_READ, &_old_protect);
#else
  unreachable();
#endif
}

void xbuf_unmap(xbuf_t *buf) {
#if ARVM_JIT_OS == ARVM_JIT_OS_POSIX
  long page_size = sysconf(_SC_PAGESIZE);
  munmap(buf->xptr, 1 + ((buf->offset - 1) / page_size));
#elif ARVM_JIT_OS == ARVM_JIT_OS_WIN32
  VirtualFree(buf->xptr, 0, MEM_RELEASE);
#else
  unreachable();
#endif
  buf->xptr = NULL;
}