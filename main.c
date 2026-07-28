#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "constants.h"

//#define WKdm
//#define lz4

#if defined WKdm
    #include "adapters/WKdm_adapter.h"
#elif defined lz4
    #include "adapters/lz4_adapter.h"
#elif defined zlib
    #include "adapters/zlib_adapter.h"
#elif defined WK64
    #include "adapters/WK64_adapter.h"
#elif defined lzo
    #include "adapters/lzo_adapter.h"
#elif defined zstd
    #include "adapters/zstd_adapter.h"
#endif

struct Record {
    int page_number;
    int iteration_number;
    int uncompressed_size;
    int compressed_size;
    time_t compression_time;
    time_t decompression_time;
};

struct Record record;


time_t calculate_duration(struct timespec start, struct timespec stop) {
    /**
     * Given a timespec start and stop, return the time elapsed in ns
     */
    uint64_t start_ns = (start.tv_sec * 1e9) + start.tv_nsec;
    uint64_t stop_ns  = (stop.tv_sec  * 1e9) + stop.tv_nsec;
    return stop_ns - start_ns;
}


void time_compression(word* src, word* dst, size_t buffer_size) {
    /**
     * Compress a src buffer of size buffer_size into the dst buffer
     */
    struct timespec start_time, stop_time;
    clock_gettime(TEST_CLOCK, &start_time);
    size_t compressed_size = compressor.compress(src, dst, buffer_size);
    clock_gettime(TEST_CLOCK, &stop_time);

    time_t duration = calculate_duration(start_time, stop_time);

    record.uncompressed_size = BYTES_PER_PAGE;
    record.compressed_size = compressed_size;
    record.compression_time = duration;
}


void time_decompression(word* src, word* dst, size_t buffer_size) {
    /**
     * Decompress the content from the dst buffer into the src buffer,
     * where the we expect the recovered data to be of size buffer_size
     */
    struct timespec start_time, stop_time;
    clock_gettime(TEST_CLOCK, &start_time);
    compressor.decompress(dst, src, buffer_size);
    clock_gettime(TEST_CLOCK, &stop_time);

    time_t duration = calculate_duration(start_time, stop_time);
    record.decompression_time = duration;
}


int main(int argc, char *argv[]) {
    if (! (1 <= argc && argc <= 1) ) {
        fprintf(stderr,
            "USAGE: %s",
            argv[0]);
    }

    int pages = 0;

    // create working buffers
    size_t buffer_size = BYTES_PER_PAGE * 2;
    word* src = (word*) malloc(buffer_size);
    word* dst = (word*) malloc(buffer_size);
    word* copy = (word*) malloc(buffer_size);
    if (src == NULL || dst == NULL || copy == NULL) {
        printf("ERROR: could not malloc working buffers\n");
        exit(-1);
    }

    FILE* in_stream = stdin;

    size_t pages_read;
    int page_counter = 0;
    int memcmp_res;
    // read in page images from stdin
    while (!feof(in_stream)) {
        page_counter++;
        if (pages > 0 && page_counter >= pages) {
            break;
        }
        record.page_number = page_counter;

        // clear working buffers
        memset((void*) src, -1, buffer_size);
        memset((void*) dst, -1, buffer_size);
        memset((void*) copy, -1, buffer_size);

        // read the page in
        pages_read = fread(src, BYTES_PER_PAGE, 1, in_stream);
        if (pages_read != 1) {
            if (feof(in_stream)) {
                break;
            }
            printf("ERROR: could not read image input from buffer");
            exit(-1);
        }

        // prepare a clean copy of the src for verification later
        memcpy((void*) copy, (void*) src, buffer_size);
        assert(memcmp((void*) src, (void*) copy, buffer_size) == 0);

        // actual work
        time_compression(src, dst, buffer_size);

        memset((void*) src, -1, buffer_size);

        time_decompression(src, dst, buffer_size);

        // verify that we recovered what we put in
        memcmp_res = memcmp((void*) src, (void*) copy, BYTES_PER_PAGE);
        if (memcmp_res != 0) {
            printf("\tHELP! %d bytes off; %d pages in\n", memcmp_res, page_counter);
            break;
        }

        printf("%d,%d,%d,%ld,%ld\n",
            record.page_number, 
            record.uncompressed_size, 
            record.compressed_size,
            record.compression_time, 
            record.decompression_time
        );

    }
    
}