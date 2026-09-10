#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"

#include "../deps/lzo-2.10/include/lzo/lzodefs.h"
#include "../deps/lzo-2.10/include/lzo/lzoconf.h"
#include "../deps/lzo-2.10/include/lzo/lzo1x.h"

typedef uint64_t word;

size_t compressed_size;
int init = 0;
void* wrkmem;

size_t lzo_wrapper_compress(word* src, word* dst, size_t buffer_size, int clevel) {
    if (!init) {
        lzo_init();
        init = 1;
    }

    lzo_uint out_len = (lzo_uint) buffer_size;

    void* wrkmem = malloc(LZO1X_999_MEM_COMPRESS);
    int status = lzo1x_999_compress_level(
        (const lzo_bytep) src,
        (lzo_uint) BYTES_PER_PAGE,
        (lzo_bytep) dst,
        &out_len,
        wrkmem,
        NULL,
        0,
        NULL,
        clevel
    );

    free(wrkmem);

    if (status != LZO_E_OK) {
        printf("ERROR: lzo1x_999_compress_level failed, ret=%d\n", status);
        exit(-1);
    }

    compressed_size = out_len;
    return out_len;
}

void lzo_wrapper_decompress(word* src, word* dst, size_t buffer_size) {
    int status = lzo1x_decompress(
        (lzo_bytep) src, compressed_size, 
        (lzo_bytep) dst, &compressed_size, 
        NULL
    );
    if (status != LZO_E_OK) {
        printf("ERROR: lzo_wrapper_decompress failed, ret=%d\n", status);
        exit(-1);
    }
}

compressor_t compressor = {
    .name = "lzo",
    .compress = lzo_wrapper_compress,
    .decompress = lzo_wrapper_decompress
};