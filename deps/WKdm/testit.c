#define _XOPEN_SOURCE 600
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "WKdm.h"

/* #define TESTIT_CLOCK CLOCK_MONOTONIC */
#define TESTIT_CLOCK CLOCK_PROCESS_CPUTIME_ID

#if defined (TESTIT_RANDOM_SEED)
#define SET_RANDOM_SEED srandom(TESTIT_RANDOM_SEED)
#else
#define SET_RANDOM_SEED
#endif



uint64_t
calc_duration (struct timespec start,
	       struct timespec stop) {

  const uint64_t ns_per_s = 1000000000;
  uint64_t start_in_ns = (start.tv_sec * ns_per_s) + start.tv_nsec;
  uint64_t stop_in_ns  = (stop.tv_sec  * ns_per_s) + stop.tv_nsec;
  uint64_t interval_in_ns = stop_in_ns - start_in_ns;

  return interval_in_ns;
  
}



void time_compression (WK_word*     source,
		       WK_word*     destination,
		       unsigned int words,
		       unsigned int iterations) {

  uint64_t     timings[iterations];
  unsigned int compressed_size;
  for (int i = 0; i < iterations; i += 1) {

    struct timespec  start_time;
    struct timespec  stop_time;
    clock_gettime(TESTIT_CLOCK, &start_time);
    compressed_size = WKdm_compress(source, destination, words);
    clock_gettime(TESTIT_CLOCK, &stop_time);
    timings[i] = calc_duration(start_time, stop_time);

  }

  printf("c");
  for (int i = 0; i < iterations; i += 1) {
    printf("\t%ld", timings[i]);
  }
  printf("\t%d\n", compressed_size);
  
}



void time_decompression (WK_word*     source,
			 WK_word*     destination,
			 unsigned int words,
			 unsigned int iterations) {

  uint64_t timings[iterations];
  for (int i = 0; i < iterations; i += 1) {
    
    struct timespec start_time;
    struct timespec stop_time;  
    clock_gettime(TESTIT_CLOCK, &start_time);
    WKdm_decompress(source, destination, words);
    clock_gettime(TESTIT_CLOCK, &stop_time);
    timings[i] = calc_duration(start_time, stop_time);

  }

  printf("d");
  for (int i = 0; i < iterations; i += 1) {
    printf("\t%ld", timings[i]);
  }
  printf("\n");

}

#if defined (TESTIT_DEBUG)
void hexdump (void* base, void* limit, char* message) {

  fprintf(stderr, "\nDEBUG: %s:\n", message);
  char* current = (char*)base;
  char* end     = (char*)limit;
  int i = 0;
  while (current < end) {
    if (i % 8 == 0) {
      fprintf(stderr, "\t@0x%16lx:", (uint64_t)current);
    }
    fprintf(stderr, "\t%02hhx", *current++);
    i += 1;
    if (i % 8 == 0) {
      fprintf(stderr, "\n");
    }
  }

  fflush(stderr);
  
}
#else
void hexdump (void* base, void* limit, char* message) {}
#endif /* TESTIT_DEBUG */

void compare (WK_word* source, WK_word* destination, unsigned int words) {

  /* Make a copy of the source page so that we have a version of the source that is guaranteed to be unmolested. */
  WK_word* copy = (WK_word*)malloc(sizeof (WK_word) * words);
  memcpy((void*)copy, (void*)source, sizeof(WK_word) * words);
  assert(memcmp((void*)source, (void*)copy, sizeof(WK_word) * words) == 0);

  /*
   * Compress and then decompress the data.  In between, clear the copy space into which decompression will occur, just to minimize
   * the effects of any stale data sitting there if the algorithm is faulty.
   */
  WKdm_compress(source, destination, words);
  memset((void*)copy, -1, words * WK_BYTES_PER_WORD);
  WKdm_decompress(destination, copy, words);

  /* Compare the decompressed data to the original. */
  if (memcmp ((void*)source, (void*)copy, sizeof(WK_word) * words) != 0) {
    unsigned int index;
    unsigned int num_mismatches = 0;
    printf ("ERROR: Compressed/decompressed did not match original\n");
    printf ("\t\tindex\tsource\t\t\tcopy\n");
    for (index = 0; index < words; index++) {
      WK_word  source_word = source[index];
      intptr_t source_word_addr = (intptr_t)&source[index];
      WK_word  copy_word = copy[index];
      intptr_t copy_word_addr = (intptr_t)&copy[index];
      if (source_word != copy_word) {
	num_mismatches += 1;
	printf("\t%4x:\t0x%8x [@0x%16lx]\t0x%8x [@0x%16lx]",
	       index,
	       source_word,
	       source_word_addr,
	       copy_word,
	       copy_word_addr);
      }
    }
    printf("\tTotal number of mismatches: %u / 0x%x\n", num_mismatches, num_mismatches);
  }

}


void testit (WK_word*     source,
	     WK_word*     destination,
	     unsigned int words,
	     unsigned int iterations) {

  compare(source, destination, words);
  time_compression  (source,      destination, words, iterations);
  time_decompression(destination, source,      words, iterations);

}



void test_images (unsigned int words, unsigned int iterations, char* image_path) {

  FILE* in_stream = (strcmp(image_path, "-") == 0 ? stdin : fopen(image_path, "r"));
  if (in_stream == NULL) {
    perror("ERROR: Could not open image input stream");
  }
  
  WK_word* a = (WK_word*)malloc(sizeof(WK_word) * words);
  WK_word* b = (WK_word*)malloc(sizeof(WK_word) * words * 4);

  uint64_t buffer_number = 0;
  while (!feof(in_stream)) {

    buffer_number += 1;
    size_t result = fread(a, WK_BYTES_PER_WORD, words, in_stream);
    if (result != words) {
      if (ferror(in_stream)) {
	fprintf(stderr, "ERROR: Failure reading image input from buffer %ld\n", buffer_number);
	exit(1);
      } else if (feof(in_stream)) {
	// Hitting an end of file is the normal way to end processing.
	break;
      } else {
	fprintf(stderr, "ERROR: Unknown error reading image input from buffer %ld\n", buffer_number);
	exit(1);
      }
    }
    
    testit(a, b, words, iterations);

  }

  if (in_stream != stdin) {
    fclose(in_stream);
  }

}



void test_pattern (unsigned int words, unsigned int iterations) {

  unsigned int buffer_size = sizeof(WK_word) * words;
  WK_word*     a           = (WK_word*)malloc(buffer_size);
  WK_word*     b           = (WK_word*)malloc(buffer_size * 4);
  printf("\nZEROS:\n");
  memset(a, 0, buffer_size);
  testit(a, b, words, iterations);

  printf("\nRANDOM:\n");
  SET_RANDOM_SEED;
  for (int i = 0; i < words; i += 1) {
    a[i] = random() * (random() % 2 == 0 ? 1 : -1);
  }
#if defined (TESTIT_DEBUG)
  hexdump(a, a + words, "Assigned random page");
#endif
  testit(a, b, words, iterations);
  
  printf("\nPATTERNED:\n");
  for (unsigned int i = 0; i < words; i++) {
    switch (rand() % 4) {
    case 0:
      a[i] = 0x1234abcd;
      break;
    case 1:
      a[i] = 0x10832;
      break;
    case 2:
      a[i] = 0x1234abcd;
      break;
    case 3:
      a[i] = 0xcccc5555;
      break;
    }
  }
  testit(a, b, words, iterations);
  
}


int main (int argc, char** argv) {

  if (! (3 <= argc && argc <= 4) ) {
    fprintf(stderr,
            "USAGE: %s <bytes per buffer>\n"
            "          <timing iterations>\n"
	    "          [ <page image path> | - (stdin) ]\n",
            argv[0]);
    exit(1);
  }

  unsigned int bytes       = strtol(argv[1], NULL, 10);
  unsigned int iterations  = strtol(argv[2], NULL, 10);
  char*        image_path  = (argc == 4) ? argv[3] : NULL;

  unsigned int words = bytes / WK_BYTES_PER_WORD;
  
  struct timespec res_time;						   
  clock_getres(TESTIT_CLOCK, &res_time);
  
  printf("Word size       = %ld bytes\n", WK_BYTES_PER_WORD);
  printf("-> Words        = %d\n", words);
  printf("-> Bytes        = %d\n", bytes);
  printf("-> Iterations   = %d\n", iterations);
  printf("-> Clock res    = %u s, %u ns\n", (unsigned int)res_time.tv_sec, (unsigned int)res_time.tv_nsec);
  printf("\n");
  
  if (image_path != NULL) {
    test_images(words, iterations, image_path);
  } else {
    test_pattern(words, iterations);
  }

  return 0;

}
