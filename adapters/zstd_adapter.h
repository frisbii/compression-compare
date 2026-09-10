#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"
#include "../deps/zstd/lib/zstd.h"

size_t compressed_size;

size_t zstd_wrapper_compress(word* src, word* dst, size_t bytes, int clevel) {

    int clevel_zstd;
    switch (clevel) {
        case 1: clevel_zstd = 0; break;
        case 2: clevel_zstd = 3; break;
        case 3: clevel_zstd = 5; break;
        case 4: clevel_zstd = 8; break;
        case 5: clevel_zstd = 10; break;
        case 6: clevel_zstd = 13; break;
        case 7: clevel_zstd = 15; break;
        case 8: clevel_zstd = 18; break;
        case 9: clevel_zstd = 20; break;
    }
    compressed_size = ZSTD_compress((void*) dst, bytes, (void*) src, BYTES_PER_PAGE, clevel_zstd);
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