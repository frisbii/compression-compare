#include <stdio.h>
#include <stdlib.h>

#include "records.h"

Record records_arr[RECORDS_ARR_LEN];
int records_arr_pos = 0;

static int flush_count = 0;

extern Record record;
extern SinkType sink_type;

void flush_records_csv() {
    if (!flush_count) {
        printf(
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
        printf("%d,%d,%d,%d,%d\n",
            r.page_number,
            r.compressed_size,
            r.uncompressed_size,
            r.compression_time,
            r.decompression_time
        );
    }
}

void flush_records_sql() {
    // unimplemented. very slow! not sure whats up but the csv things currently do it well enough for me
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
