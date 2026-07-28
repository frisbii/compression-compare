#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"
#include "../deps/zstd/lib/zstd.h"

size_t compressed_size;

size_t zstd_wrapper_compress(word* src, word* dst, size_t bytes) {
    compressed_size = ZSTD_compress((void*) dst, bytes, (void*) src, BYTES_PER_PAGE, 1);
    if (ZSTD_isError(compressed_size)) {
        printf("ERROR: in ZSTD_compress\n");
        exit(-1);
    }
    return compressed_size;
}

void zstd_wrapper_decompress(word* src, word* dst, size_t bytes) {
    size_t decompressed_size = ZSTD_decompress((void*) dst, bytes, (void*) src, compressed_size);
    if (ZSTD_isError(decompressed_size)) {
        printf("ERROR: in ZSTD_decompress. code=%d : %s\n", 
            ZSTD_getErrorCode(decompressed_size),
            ZSTD_getErrorName(decompressed_size));
        exit(-1);
    }
}

compressor_t compressor = {
    .name = "zstd",
    .compress = zstd_wrapper_compress,
    .decompress = zstd_wrapper_decompress
};