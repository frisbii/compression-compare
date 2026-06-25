#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <dlfcn.h>

#include "compressor.h"

typedef uint64_t word;
#define BYTES_PER_WORD sizeof(word) // 8
#define BYTES_PER_PAGE 4096
#define WORDS_PER_PAGE 512

#define TEST_CLOCK CLOCK_PROCESS_CPUTIME_ID

int main() {
    size_t pages = 1;
    size_t buffer_size = BYTES_PER_PAGE * 2;
    word* src = (word*) malloc(buffer_size);
    word* dst = (word*) malloc(buffer_size);
    word* copy = (word*) malloc(buffer_size);

    /* char* image_path = "./ollama_qwen25_coder_32b.page-images";
    FILE* in_stream = fopen(image_path, "r");
    if (in_stream == NULL) {
        perror("ERROR: could not open image");
    } */
    FILE* in_stream = stdin;

    size_t pages_read;
    int memcmp_res;

    memset((void*) src, -1, buffer_size);
    memset((void*) dst, -1, buffer_size);
    memset((void*) copy, -1, buffer_size);

    pages_read = fread(src, BYTES_PER_PAGE, 1, in_stream);
    if (pages_read != 1) {
        printf("ERROR: could not read image input from buffer");
        exit(-1);
    }

    // prepare a clean copy of the src for verification later
    memcpy((void*) copy, (void*) src, buffer_size);
    assert(memcmp((void*) src, (void*) copy, buffer_size) == 0);


    void* handle = dlopen("adapters/lz4.so", RTLD_LAZY);
    dlerror();
    compressor_t* (*reg)(void) = dlsym(handle, "register_compressor"); 
    char* error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Symbol extraction failed: %s\n", error);
        dlclose(handle);
        exit(-1);
    }

    compressor_t* compressor = reg();






    /* int compressed_size = LZ4_compress_default((char*) src, (char*) dst, (int) BYTES_PER_PAGE, (int) buffer_size);
    printf("compressed. size=%d\n", compressed_size);

    int decomp_size = LZ4_decompress_safe((char*) dst, (char*) src, compressed_size, BYTES_PER_PAGE);
    printf("decompressed. status=%d size=%d\n", decomp_size); */
    



    /* time_compression(src, dst, BYTES_PER_PAGE);

    memset((void*) src, -1, BYTES_PER_PAGE);

    time_decompression(src, dst, BYTES_PER_PAGE);

    // checks
    memcmp_res = memcmp((void*) src, (void*) copy, BYTES_PER_PAGE);
    if (memcmp_res != 0) {
        printf("\tHELP! %d bytes off; %d pages in\n", memcmp_res, page_counter);
        break;
    } */
    
}