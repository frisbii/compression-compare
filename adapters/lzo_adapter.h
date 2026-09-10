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

size_t lzo_wrapper_compress(word* src, word* dst, size_t buffer_size, int clevel) {
    compressed_size = buffer_size;
    if (!init) {
        lzo_init();
        init = 1;
    }

    void* wrkmem;
    int status;
    if (clevel == 0) {
        wrkmem = malloc(LZO1X_1_MEM_COMPRESS * 2);
        status = lzo1x_1_compress(
            (lzo_bytep) src, BYTES_PER_PAGE, 
            (lzo_bytep) dst, &compressed_size, 
            wrkmem
        );
    } else {
        // its never really explained what the exposed clevel here is but it is 1-9
        wrkmem = malloc(LZO1X_999_MEM_COMPRESS);
        status = lzo1x_999_compress_level(
            (lzo_bytep) src, BYTES_PER_PAGE,
            (lzo_bytep) dst, &compressed_size,
            wrkmem,
            NULL,
            0,
            NULL,
            clevel
        );
    }

    free(wrkmem);

    if (status != LZO_E_OK) {
        printf("ERROR: lzo1x_1_compress failed");
        exit(-1);
    }

    return compressed_size;
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