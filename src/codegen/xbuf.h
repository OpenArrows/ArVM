#include <stddef.h>

#define XBUF_BLOCK_SIZE 1024

typedef struct xbuf_block_header {
  struct xbuf_block *prev;
  size_t offset;
} xbuf_block_header_t;

typedef struct xbuf_block {
  xbuf_block_header_t header;
  char data[XBUF_BLOCK_SIZE - sizeof(xbuf_block_header_t)];
} xbuf_block_t;

typedef struct xbuf {
  void *xptr;

  xbuf_block_t *tail;
  size_t offset;
} xbuf_t;

void *xbuf_write(xbuf_t *buf, void *data, size_t length);

void xbuf_free(xbuf_t *buf);

void xbuf_map(xbuf_t *buf);

void xbuf_unmap(xbuf_t *buf);