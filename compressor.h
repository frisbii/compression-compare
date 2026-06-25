#include <stdint.h>
#include <string.h>

#include "constants.h"

typedef struct {
    const char *name;
    size_t (*compress)(word* src, word* dst, size_t buffer_size);
        // src contains the original data. dst is the compressed form.
        // buffer_size is the size of the dst buffer. returns size of dst.
    void (*decompress)(word* src, word* dst, size_t buffer_size);
        // src contains the compressed data. dst is the recovered form.
        // buffer_size is the size of the src buffer.
} compressor_t;