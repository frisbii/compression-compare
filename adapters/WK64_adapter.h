#include <stdint.h>
#include <stdlib.h>

#include "../constants.h"
#include "../compressor.h"

#define WK_DICTIONARY_SIZE 16
#include "../deps/WK64/WK.h"


size_t WK64_wrapper_compress(word* src, word* dst, size_t buffer_size) {
    WK_packing_word* end = COMPRESS_FUNC((WK_modeling_word*) src,
        (WK_modeling_word*) dst, BYTES_PER_PAGE / BYTES_PER_WORD);
    size_t compressed_size = (end - dst) * sizeof(WK_packing_word);
    return compressed_size;
}

void WK64_wrapper_decompress(word* src, word* dst, size_t buffer_size) {
    DECOMPRESS_FUNC ((WK_modeling_word*) src, (WK_modeling_word*) dst);
}

compressor_t compressor = {
    .name = "WKdm",
    .compress = WK64_wrapper_compress,
    .decompress = WK64_wrapper_decompress
};