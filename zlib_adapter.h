#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "compressor.h"
#include "deps/zlib-1.3.2/zlib.h"

size_t zlib_wrapper_compress(word* src, word* dst, size_t bytes) {
    int status = compress_z((Bytef*) dst, &bytes, (Bytef*) src, bytes);
    if (status != 0) {
        printf("ERROR: in call to compress_z, ret value %d\n", status);
        exit(-1);
    }
    return bytes;
}

void zlib_wrapper_decompress(word* src, word* dst, size_t bytes) {
    int status = uncompress_z((Bytef*) dst, &bytes, (Bytef*) src, bytes);
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