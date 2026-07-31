#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <time.h>
#include <immintrin.h>

#include "constants.h"

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

typedef enum {
    NONE,
    CLFLUSH,
    LARGE_ARR,
    RAND_LARGE_ARR
} InvalidationMethod;

typedef enum {
    CSV,
    SQL
} SinkType;

SinkType sink_type;
int flush_count = 0;

typedef struct {
    int iterations;
    InvalidationMethod invalidation_method;

    int page_number;
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
    for (; pos < end; pos += 1) {
        _mm_clflush(pos);
    }
    _mm_mfence();
}



static volatile unsigned char* cache_sweep_buffer = NULL;
static size_t cache_sweep_size = 64 * 1024 * 1024;  // 64 MB

void invalidate_cache_touch_large_array(word* buf, size_t buffer_size) {
    (void) buf;
    (void) buffer_size;

    if (cache_sweep_buffer == NULL) {
        void* raw = NULL;
        if (posix_memalign(&raw, 64, cache_sweep_size) != 0) {
            fprintf(stderr, "ERROR: could not allocate cache sweep buffer\n");
            exit(1);
        }
        cache_sweep_buffer = (volatile unsigned char*) raw;
    }

    for (size_t i = 0; i < cache_sweep_size; i += 64) {
        cache_sweep_buffer[i] = (unsigned char) (i + 1);
    }

    _mm_mfence();
}

static volatile long *rand_buffer = NULL;
static size_t rand_sweep_size = 64 * 1024 * 1024;  // 64 MB

void invalidate_cache_rand_large_array(word* buf, size_t buffer_size) {
    if (rand_buffer == NULL) {
        void* raw = NULL;
        if (posix_memalign(&raw, 64, rand_sweep_size) != 0) {
            fprintf(stderr, "ERROR: could not allocate cache sweep buffer\n");
            exit(1);
        }
        rand_buffer = (volatile long*) raw;
    }

    for (size_t i = 0; i < cache_sweep_size; i++) {
        rand_buffer[i] = rand();
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
    case LARGE_ARR:
        invalidate_cache_touch_large_array(buf, buffer_size);
        break;
    case RAND_LARGE_ARR:
        invalidate_cache_rand_large_array(buf, buffer_size);
        break;
    default:
        fprintf(stderr, "unknown invalidation method\n");
        exit(1);
    }
}



void flush_records_csv() {
    if (!flush_count) {
        printf(
            "iterations,"
            "invalidation_method,"

            "page_number,"
            "compressed_size,"
            "uncompressed_size,"
            "compression_time,"
            "decompression_time"
            "\n"
        );
    }

    for (int i = 0; i < records_arr_pos; i++) {
        Record r = records_arr[i];
        printf("%d,%d,%d,%d,%d,%d,%d\n",
            r.iterations,
            r.invalidation_method,

            r.page_number,
            r.compressed_size,
            r.uncompressed_size,    
            r.compression_time,
            r.decompression_time
        );
    }
}

void flush_records_sql() {
    /* if (!flush_count) {
        printf(
            "CREATE TABLE IF NOT EXISTS measurements (\n"
            "  page_number INTEGER,\n"
            "  compressed_size INTEGER,\n"
            "  uncompressed_size INTEGER,\n"
            "  compression_time INTEGER,\n"
            "  decompression_time INTEGER\n"
            ");\n"
        );
    }

    printf("BEGIN TRANSACTION;\n");
    for (int i = 0; i < records_arr_pos; i++) {
        Record r = records_arr[i];
        printf(
            "INSERT INTO measurements ("
            "page_number, "
            "compressed_size, "
            "uncompressed_size, "
            "compression_time, "
            "decompression_time"
            ") VALUES (%d, %d, %d, %d, %d);\n",

            r.page_number,
            r.compressed_size,
            r.uncompressed_size,
            r.compression_time,
            r.decompression_time
        );
    }
    printf("COMMIT;\n"); */
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
        fprintf(stderr, "Unknown sink type\n");
        exit(1);
}
    flush_count++;
}



void add_record() {
    if (records_arr_pos >= RECORDS_ARR_LEN) {
        flush_records();
        records_arr_pos = 0;
    }
    records_arr[records_arr_pos++] = record;
}




int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr,
            "USAGE: %s COMPRESS_INVALIDATION SINK_TYPE record.iterations\n",
            argv[0]);
        fprintf(stderr, "  Invalidation options: none|clflush \n");
        fprintf(stderr, "  Sink options: csv|sql\n");
        return 1;
    }

    const char *inv_arg = argv[1];
    const char *sink_arg = argv[2];
    record.iterations = atoi(argv[3]);

    // parse invalidation method
    if (strcmp(inv_arg, "none") == 0) {
        record.invalidation_method = NONE;
    } else if (strcmp(inv_arg, "clflush") == 0) {
        record.invalidation_method = CLFLUSH;
    } else if (strcmp(inv_arg, "large_arr") == 0) {
        record.invalidation_method = LARGE_ARR;
    } else if (strcmp(inv_arg, "rand_large_arr") == 0) {
        record.invalidation_method = RAND_LARGE_ARR;
    } else {
        fprintf(stderr, "unknown cache invalidation method: %s\n", inv_arg);
        exit(1);
    }

    // parse sink type
    if (strcmp(sink_arg, "csv") == 0) {
        sink_type = CSV;
    } else if (strcmp(sink_arg, "sql") == 0) {
        sink_type = SQL;
    } else {
        fprintf(stderr, "unknown sink: %s\n", sink_arg);
        exit(1);
    }

    int max_pages = 0;

    // create working buffers
    size_t buffer_size = BYTES_PER_PAGE * 2;

    // use memalign to ensure page alignment
    word *src, *dst, *copy;
    posix_memalign((void**) &src, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**) &dst, BYTES_PER_PAGE, buffer_size);
    posix_memalign((void**) &copy, BYTES_PER_PAGE, buffer_size);
    if (src == NULL || dst == NULL || copy == NULL) {
        fprintf(stderr, "ERROR: could not malloc working buffers\n");
        exit(1);
    }

    // read in page images from stdin
    FILE* in_stream = stdin;
    int page_count = 0;

    while (!feof(in_stream)) {
        page_count++;
        if (max_pages > 0 && page_count >= max_pages) {
            break;
        }

        // store params in record
        record.page_number = page_count;

        // clear working buffers
        memset((void*) src, -1, buffer_size);
        memset((void*) dst, -1, buffer_size);
        memset((void*) copy, -1, buffer_size);

        // read the page in
        size_t pages_read = fread(src, BYTES_PER_PAGE, 1, in_stream);
        if (pages_read != 1) {
            if (feof(in_stream)) {
                break;
            }
            fprintf(stderr, "ERROR: could not read image input from buffer");
            exit(1);
        }

        // prepare a clean copy of the src for verification later
        memcpy((void*) copy, (void*) src, buffer_size);
        assert(memcmp((void*) src, (void*) copy, buffer_size) == 0);

        /////////////////////////////////////////////////////////////////
        int total_comp_time = 0;
        int total_decomp_time = 0;
        for (int i = 0; i < record.iterations; i++) {
            // invalidate src after copying
            invalidate_cache(record.invalidation_method, src, buffer_size);
            
            // compress src into dst
            time_compression(src, dst, buffer_size);
            
            // erase content in src
            memset((void*) src, -1, buffer_size);
            
            // invalidate dst after compressing into it
            invalidate_cache(record.invalidation_method, dst, buffer_size);
            
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

        record.compression_time = total_comp_time / record.iterations;
        record.decompression_time = total_decomp_time / record.iterations;

        // store results
        add_record();
    }

    flush_records();
    
}