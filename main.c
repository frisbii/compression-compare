#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <immintrin.h>

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

typedef enum {
    NONE,
    CLFLUSH
} InvalidationMethod;

InvalidationMethod invalidation_method_compress;
InvalidationMethod invalidation_method_decompress;

typedef enum {
    CSV,
    SQL
} SinkType;

SinkType sink_type;
int flush_count = 0;

typedef struct {
    int page_number;
    int iteration_number;

    InvalidationMethod cache_invalidation_method_compress;
    InvalidationMethod cache_invalidation_method_decompress;

    int uncompressed_size;
    int compressed_size;
    int compression_time;
    int decompression_time;
} Record;

#define RECORDS_ARR_LEN 10000
Record record;
Record records_arr[RECORDS_ARR_LEN];
int records_arr_pos = 0;


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



void invalidate_cache_clflush(word* buf, size_t buffer_size) {
    word* end = buf + buffer_size;
    
    const int line_size = 64; 
    
    word* buf_align = (word*) ((uintptr_t) buf & ~((uintptr_t) line_size - 1));
    assert(buf == buf_align);
    
    word* pos = buf;
    _mm_sfence();
    for (; pos < end; pos += line_size) {
        _mm_clflush(pos);
    }
    _mm_mfence();
}



void invalidate_cache(InvalidationMethod method, word* buf, size_t buffer_size) {
    switch (method) {
        case NONE:
            break;
        case CLFLUSH:
            invalidate_cache_clflush(buf, buffer_size);
            break;
        default:
            printf("Help!");
            exit(1);
    }
}



void flush_records_csv() {
    if (!flush_count) {
        printf(
            "cache_invalidation_method_compress,"
            "cache_invalidation_method_decompress,"

            "page_number,"
            "iteration_number,"

            "compressed_size,"
            "uncompressed_size,"

            "compression_time,"
            "decompression_time"
            "\n"
        );
    }

    Record this_record;
    for (int i = 0; i < records_arr_pos; i++) {
        this_record = records_arr[i];
        printf("%d,%d,%d,%d,%d,%d,%d,%d\n",
            this_record.cache_invalidation_method_compress,
            this_record.cache_invalidation_method_decompress,
    
            this_record.page_number,
            this_record.iteration_number,
            
            this_record.compressed_size,
            this_record.uncompressed_size,    
            
            this_record.compression_time,
            this_record.decompression_time
        );
    }

    flush_count++;
}

void flush_records_sql() {

}



void flush_records() {
    switch (sink_type) {
        case CSV:
            flush_records_csv();
            break;
        case SQL:
            flush_records_sql();
            break;
        default:
            printf("Help!");
            exit(1);
    }
}



void add_record() {
    if (records_arr_pos >= RECORDS_ARR_LEN) {
        flush_records();
        records_arr_pos = 0;
    }
    records_arr[records_arr_pos++] = record;
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

    word *src, *dst, *copy;
    // ensure page alignment
    posix_memalign((void**) &src, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**) &dst, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**) &copy, BYTES_PER_PAGE, buffer_size);
    if (src == NULL || dst == NULL || copy == NULL) {
        printf("ERROR: could not malloc working buffers\n");
        exit(-1);
    }

    FILE* in_stream = stdin;

    size_t pages_read;
    int page_counter = 0, iteration_counter = 0;
    int memcmp_res;

    // params
    invalidation_method_compress = NONE;
    invalidation_method_decompress = NONE;
    sink_type = CSV;

    // read in page images from stdin
    while (!feof(in_stream)) {
        page_counter++;
        if (pages > 0 && page_counter >= pages) {
            break;
        }

        // store params in record
        record.page_number = page_counter;
        record.iteration_number = iteration_counter;
        record.cache_invalidation_method_compress = invalidation_method_compress;
        record.cache_invalidation_method_decompress = invalidation_method_decompress;

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

        /////////////////////////////////////////////////////////////////
        // invalidate src after copying
        invalidate_cache(invalidation_method_compress, src, buffer_size);
        
        // compress src into dst
        time_compression(src, dst, buffer_size);
        
        // erase content in src
        memset((void*) src, -1, buffer_size);
        
        // invalidate dst after compressing into it
        invalidate_cache(invalidation_method_decompress, dst, buffer_size);
        
        // decompress dst into src
        time_decompression(src, dst, buffer_size);

        // verify that we recovered what we put in
        memcmp_res = memcmp((void*) src, (void*) copy, BYTES_PER_PAGE);
        if (memcmp_res != 0) {
            printf("\tHELP! %d bytes off; %d pages in\n", memcmp_res, page_counter);
            break;
        }

        add_record();
    }

    flush_records();
    
}