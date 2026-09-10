#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <time.h>
#include <immintrin.h>

#include "constants.h"
#include "records.h"

#if defined WKdm
    #include "adapters/WKdm_adapter.h"
    char* algdef = "WKdm";
#elif defined lz4
    #include "adapters/lz4_adapter.h"
    char* algdef = "lz4";
#elif defined zlib
    #include "adapters/zlib_adapter.h"
    char* algdef = "zlib";
#elif defined WK64
    #include "adapters/WK64_adapter.h"
    char* algdef = "WK64";
#elif defined lzo
    #include "adapters/lzo_adapter.h"
    char* algdef = "lzo";
#elif defined zstd
    #include "adapters/zstd_adapter.h"
    char* algdef = "zstd";
#endif

typedef struct {
    uint8_t ref_type_tag;
    uint64_t address_space_id;
    uint64_t page_id;
} PageImageMetadata;

SinkType sink_type;
Record record;

int OLD_IMAGE_FORMAT = 1;
int DEBUG_LEVEL = 0;

// benchmark

time_t calculate_duration(struct timespec start, struct timespec stop) {
    /**
     * Given a timespec start and stop, return the time elapsed in ns
     */
    uint64_t start_ns = (start.tv_sec * 1e9) + start.tv_nsec;
    uint64_t stop_ns  = (stop.tv_sec  * 1e9) + stop.tv_nsec;
    return stop_ns - start_ns;
}

void time_compression(word* src, word* dst, size_t buffer_size, int clevel) {
    /**
     * Compress a src buffer of size buffer_size into the dst buffer
     */
    struct timespec start_time, stop_time;
    clock_gettime(TEST_CLOCK, &start_time);
    size_t compressed_size = compressor.compress(src, dst, buffer_size, clevel);
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



void show_usage_and_exit(char *exe) {
    fprintf(stderr, "USAGE: %s COMPRESSION_LEVEL CACHE_INVAL_METHOD SINK_TYPE ITERATIONS\n", exe);
    fprintf(stderr, "  Compression level: 1 (fast) - 9 (small) \n");
    fprintf(stderr, "  Invalidation options: none|clflush \n");
    fprintf(stderr, "  Sink options: csv|sql\n");
    fprintf(stderr, "  Iterations: int\n");
    exit(1);
}



int main(int argc, char *argv[]) {
    if (argc != 5) {
        show_usage_and_exit(argv[0]);
    }

    int clevel = atoi(argv[1]);
    const char *inv_arg = argv[2];
    const char *sink_arg = argv[3];
    int iterations = atoi(argv[4]);

    // verify clevel
    if (!((1 <= clevel) && (clevel <= 9))) {
        fprintf(stderr, "invalid compression level: %d\n", clevel);
        show_usage_and_exit(argv[0]);
    }

    // parse invalidation method
    InvalidationMethod invalidation_method;
    if (strcmp(inv_arg, "none") == 0) {
        invalidation_method = NONE;
    } else if (strcmp(inv_arg, "clflush") == 0) {
        invalidation_method = CLFLUSH;
    } else if (strcmp(inv_arg, "largearr") == 0) {
        invalidation_method = LARGEARR;
    } else {
        fprintf(stderr, "unknown cache invalidation method: %s\n", inv_arg);
        show_usage_and_exit(argv[0]);
    }

    // parse sink type
    if (strcmp(sink_arg, "csv") == 0) {
        sink_type = CSV;
    } else if (strcmp(sink_arg, "sql") == 0) {
        sink_type = SQL;
    } else {
        fprintf(stderr, "unknown sink: %s\n", sink_arg);
        show_usage_and_exit(argv[0]);
    }

    // verify iterations
    if (iterations < 1) {
        fprintf(stderr, "invalid iterations input: %d\n", iterations);
        show_usage_and_exit(argv[0]);
    }

    int max_pages = 0;

    // create working buffers
    size_t buffer_size = BYTES_PER_PAGE * 2;

    // use memalign to ensure page alignment
    word *src, *dst, *copy;
    posix_memalign((void**)  &src, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**)  &dst, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**) &copy, BYTES_PER_PAGE, buffer_size);
    if (src == NULL || dst == NULL || copy == NULL) {
        fprintf(stderr, "ERROR: could not malloc working buffers\n");
        exit(1);
    }

    // read in page images from stdin
    FILE* in_stream = stdin;
    int page_count = 0;
    PageImageMetadata page_image_metadata;

    while (!feof(in_stream)) {
        page_count++;
        if (max_pages > 0 && page_count >= max_pages) {
            break;
        }

        // store params in record
        record.page_number = page_count;

        // clear working buffers
        memset((void*)  src, -1, buffer_size);
        memset((void*)  dst, -1, buffer_size);
        memset((void*) copy, -1, buffer_size);

        // read the page image
        //      read the metadata
        if (!OLD_IMAGE_FORMAT) {
            if (fread(&page_image_metadata, sizeof(PageImageMetadata), 1, in_stream) != 1) {
                if (feof(in_stream)) {
                    break;
                }
                fprintf(stderr, "ERROR: could not read image input from buffer (failed to get metadata)");
                exit(1);
            }
        }
        //      read the actual page
        if (fread(src, BYTES_PER_PAGE, 1, in_stream) != 1) {
            if (feof(in_stream)) {
                break;
            }
            fprintf(stderr, "ERROR: could not read image input from buffer (got metadata, but no image)");
            exit(1);
        }

        if (DEBUG_LEVEL >= 2) {
            printf("%X\t%X\t%X\n", page_image_metadata.ref_type_tag, page_image_metadata.address_space_id, page_image_metadata.page_id);
        }

        // prepare a clean copy of the src for verification later
        memcpy((void*) copy, (void*) src, buffer_size);
        assert(memcmp((void*) src, (void*) copy, buffer_size) == 0);

        /////////////////////////////////////////////////////////////////
        int total_comp_time = 0;
        int total_decomp_time = 0;
        for (int i = 0; i < iterations; i++) {
            // invalidate src after copying
            invalidate_cache(invalidation_method, src, buffer_size);
            
            // compress src into dst
            time_compression(src, dst, buffer_size, clevel);
            
            // erase content in src
            memset((void*) src, -1, buffer_size);
            
            // invalidate dst after compressing into it
            invalidate_cache(invalidation_method, dst, buffer_size);
            
            // decompress dst into src
            time_decompression(src, dst, buffer_size);
    
            // verify that we recovered what we put in
            int memcmp_res = memcmp((void*) src, (void*) copy, BYTES_PER_PAGE);
            if (memcmp_res != 0) {
                fprintf(stderr, "\tHELP! %d bytes off; %d pages in\n", memcmp_res, page_count);
                exit(1);
            }

            total_comp_time += record.compression_time;
            total_decomp_time += record.decompression_time;
        }

        record.compression_time = total_comp_time / iterations;
        record.decompression_time = total_decomp_time / iterations;

        // store results
        if (DEBUG_LEVEL == 0) {
            add_record();
        }
    }

    if (DEBUG_LEVEL == 0) {
        flush_records();
    }
    
}