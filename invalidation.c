#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <immintrin.h>
#include <stdint.h>

#include "invalidation.h"

static volatile unsigned char* cache_sweep_buffer = NULL;
static size_t cache_sweep_size = 40 * 1024 * 1024;  // lscpu reports 22MiB l3, so allocate approx double that

void invalidate_cache_clflush(void* buf, size_t buffer_size) {
    const int line_size = 64;
    uintptr_t addr = (uintptr_t) buf;
    void* buf_align = (void*) (addr & ~((uintptr_t) line_size - 1));
    assert(buf == buf_align);

    volatile char* pos = (volatile char*) buf;
    _mm_sfence();
    for (; pos < (volatile char*) ((char*)buf + buffer_size); pos += 1) {
        _mm_clflush((void*) pos);
    }
    _mm_mfence();
}

void invalidate_cache_largearray(void) {
    if (cache_sweep_buffer == NULL) {
        void* raw = NULL;
        if (posix_memalign(&raw, 64, cache_sweep_size) != 0) {
            fprintf(stderr, "could not allocate cache sweep buffer\n");
            exit(1);
        }
        cache_sweep_buffer = (volatile unsigned char*) raw;
    }

    for (size_t i = 0; i < cache_sweep_size; i += 64) {
        cache_sweep_buffer[i] = (unsigned char) (i + 1);
    }

    _mm_mfence();
}

void invalidate_cache(InvalidationMethod method, void* buf, size_t buffer_size) {
    switch (method) {
    case NONE:
        break;
    case CLFLUSH:
        invalidate_cache_clflush(buf, buffer_size);
        break;
    case LARGEARR:
        invalidate_cache_largearray();
        break;
    default:
        fprintf(stderr, "unknown invalidation method\n");
        exit(1);
    }
}
