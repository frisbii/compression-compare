#ifndef RECORDS_H
#define RECORDS_H

#include <stdint.h>
#include "invalidation.h"

typedef enum {
    CSV,
    SQL
} SinkType;

typedef struct {
    int iterations;
    int clevel;
    InvalidationMethod invalidation_method;

    int page_number;
    int uncompressed_size;
    int compressed_size;
    int compression_time;
    int decompression_time;
} Record;

#define RECORDS_ARR_LEN 10000

extern Record record;
extern SinkType sink_type;

extern Record records_arr[RECORDS_ARR_LEN];
extern int records_arr_pos;

void flush_records_csv(void);
void flush_records_sql(void);
void flush_records(void);
void add_record(void);

#endif
