#include <stdint.h>

typedef uint64_t word;
#define BYTES_PER_WORD sizeof(word) // 8
#define BYTES_PER_PAGE 4096
#define WORDS_PER_PAGE 512
#define BUFFER_SIZE (2 * BYTES_PER_PAGE)

#define TEST_CLOCK CLOCK_PROCESS_CPUTIME_ID