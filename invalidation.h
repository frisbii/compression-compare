#ifndef INVALIDATION_H
#define INVALIDATION_H

#include <stddef.h>

typedef enum {
    NONE,
    CLFLUSH,
    LARGEARR
} InvalidationMethod;

void invalidate_cache_clflush(void* buf, size_t buffer_size);
void invalidate_cache_largearray(void);
void invalidate_cache(InvalidationMethod method, void* buf, size_t buffer_size);

#endif