#include <stdio.h>
#include <stdint.h>

void main() {

    uint64_t *src;
    int pi_record_len = 4113;
    if (posix_memalign((void**) &src, 4096, pi_record_len) != 0) {
        fprintf(stderr, "failed to allocate space for buffer\n");
        exit(1);
    }

    FILE* in_stream = stdin;
    while (!feof(in_stream)) {
        /* size_t bytes_read = fread(src, 1, pi_record_len, in_stream);
        if (bytes_read < pi_record_len) {
            if (feof(in_stream)) {
                break;
            }
            fprintf(stderr, "failed to read page image record. read %d bytes of %d attempted\n", bytes_read, pi_record_len);
        } */

        uint8_t ref_type_tag;
        uint64_t address_space_id;
        uint64_t page_id;
        char page_image[4096];

        fread(&ref_type_tag, 1, 1, in_stream);
        fread(&address_space_id, 1, 8, in_stream);
        fread(&page_id, 1, 8, in_stream);
        fread(&page_image, 1, 4096, in_stream);

        printf("%X\t\t%X\t\t%X\n", ref_type_tag, address_space_id, page_id);


    }
}