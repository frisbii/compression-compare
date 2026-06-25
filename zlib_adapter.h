#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "compressor.h"
#include "deps/zlib-1.3.2/zlib.h"

size_t zlib_wrapper_compress(word* src, word* dst) {
    size_t dst_size = BUFFER_SIZE;
    int status = compress_z((Bytef*) dst, &dst_size, (Bytef*) src, BYTES_PER_PAGE);
    if (status != 0) {
        printf("ERROR: in call to compress_z, ret value %d\n", status);
        exit(-1);
    }
    return dst_size;
}

void zlib_wrapper_decompress(word* src, word* dst, size_t compressed_size) {
    size_t src_size = BYTES_PER_PAGE;
    int status = uncompress_z((Bytef*) dst, &src_size, (Bytef*) src, compressed_size);
    if (status != 0) {
        printf("ERROR: in call to uncompress_z, ret value %d\n", status);
        exit(-1);
    }
}

compressor_t compressor = {
    .name = "zlib",
    .compress = zlib_wrapper_compress,
    .decompress = zlib_wrapper_decompress
};