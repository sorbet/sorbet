#ifndef PRISM_XALLOCATOR_H
#define PRISM_XALLOCATOR_H

// Prism's allocator hooks (see `PRISM_XALLOCATOR` in prism/defines.h). Prism includes this header, so it has to stay
// plain C. The functions live in parser/prism/xallocator.cc: while a parse is running they hand out blocks from that
// parse's arena (parser/prism/PrismArena.h), otherwise they fall back to the system allocator.

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *sorbet_prism_xmalloc(size_t size);
void *sorbet_prism_xcalloc(size_t count, size_t size);
void *sorbet_prism_xrealloc(void *ptr, size_t size);
void sorbet_prism_xfree(void *ptr);

#ifdef __cplusplus
}
#endif

#define xmalloc sorbet_prism_xmalloc
#define xrealloc sorbet_prism_xrealloc
#define xcalloc sorbet_prism_xcalloc
#define xfree sorbet_prism_xfree

#endif // PRISM_XALLOCATOR_H
