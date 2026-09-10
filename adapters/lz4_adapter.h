#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"
#include "../deps/lz4/lib/lz4.h"

typedef uint64_t word;

int compressed_size;

size_t lz4_wrapper_compress(word* src, word* dst, size_t buffer_size, int clevel) {
    //compressed_size = LZ4_compress_default((char*) src, (char*) dst, BYTES_PER_PAGE, buffer_size);

    int clevel_lz4;
    switch (clevel) {
        case 9: clevel_lz4 = 0; break;
        case 8: clevel_lz4 = 8191; break;
        case 7: clevel_lz4 = 16383; break;
        case 6: clevel_lz4 = 24575; break;
        case 5: clevel_lz4 = 32767; break;
        case 4: clevel_lz4 = 40959; break;
        case 3: clevel_lz4 = 49151; break;
        case 2: clevel_lz4 = 57343; break;
        case 1: clevel_lz4 = 65535; break;
    }

    compressed_size = LZ4_compress_fast((char*) src, (char*) dst, BYTES_PER_PAGE, buffer_size, clevel_lz4);
    if (compressed_size < 0) {
        printf("ERROR: LZ4_compress_fast failed\n");
        exit(-1);
    }
    return compressed_size;
}

void lz4_wrapper_decompress(word* src, word* dst, size_t buffer_size) {
    int decompressed_size = LZ4_decompress_safe((char*) src, (char*) dst, compressed_size, BYTES_PER_PAGE);
    if (decompressed_size < 0) {
        printf("ERROR: LZ4_decompress_safe failed\n");
        exit(-1);
    }
}

compressor_t compressor = {
    .name = "lz4",
    .compress = lz4_wrapper_compress,
    .decompress = lz4_wrapper_decompress
};