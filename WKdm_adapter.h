#include <stdint.h>
#include <stdlib.h>

#include "constants.h"
#include "compressor.h"
#include "deps/WKdm/WKdm.h"

#define WKdm_BYTES_PER_WORD 4

size_t WKdm_wrapper_compress(word* src, word* dst, size_t buffer_size) {
    return (size_t) WKdm_compress((WK_word*) src, (WK_word*) dst, BYTES_PER_PAGE / WKdm_BYTES_PER_WORD);
}

void WKdm_wrapper_decompress(word* src, word* dst, size_t buffer_size) {
    WKdm_decompress((WK_word*) src, (WK_word*) dst, BYTES_PER_PAGE / WKdm_BYTES_PER_WORD);
}

compressor_t compressor = {
    .name = "WKdm",
    .compress = WKdm_wrapper_compress,
    .decompress = WKdm_wrapper_decompress
};