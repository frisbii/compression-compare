#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"
#include "../deps/lz4/lib/lz4.h"

typedef uint64_t word;

int compressed_size;

size_t lz4_wrapper_compress(word* src, word* dst, size_t buffer_size) {
    compressed_size = LZ4_compress_default((char*) src, (char*) dst, BYTES_PER_PAGE, buffer_size);
    if (compressed_size < 0) {
        printf("ERROR: LZ4_compress_default failed\n");
        exit(-1);
    }
    return compressed_size;
}

void lz4_wrapper_decompress(word* src, word* dst, size_t buffer_size) {
    int status = LZ4_decompress_safe((char*) src, (char*) dst, compressed_size, BYTES_PER_PAGE);
    if (status < 0) {
        printf("ERROR: LZ4_decompress_safe failed\n");
        exit(-1);
    }
}

compressor_t compressor = {
    .name = "lz4",
    .compress = lz4_wrapper_compress,
    .decompress = lz4_wrapper_decompress
};