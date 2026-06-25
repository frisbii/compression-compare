#include <stdint.h>
#include <string.h>

#include "constants.h"

typedef struct {
    const char *name;
    size_t (*compress)(word* src, word* dst);
        // src contains the original data. dst is the compressed form.
        // returns size of dst.
    void (*decompress)(word* src, word* dst, size_t compressed_size);
        // src contains the compressed data. dst is the recovered form.
} compressor_t;