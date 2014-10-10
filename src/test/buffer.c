#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../util/stringutil.h"

#define ITER 1000000
#define SAMPLE_STRING_SHORT "Hello world!"
#define SAMPLE_STRING_LONG "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
							\aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

int main(void) {
	Buffer *buff = NULL;
	char *strbuff, *str = NULL;
	struct timespec start, end;
	double elaps;	
	long iteration;

	printf("# String duplication\n");

	str = stringDuplication(SAMPLE_STRING_SHORT);

	if (strcmp(str, SAMPLE_STRING_SHORT) != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string duplication (%d iteration on %zu bytes)\n", ITER, strlen(SAMPLE_STRING_LONG));

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITER; iteration++) {

		str = stringDuplication(SAMPLE_STRING_LONG);

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %fs.\n", elaps);

	printf("# String n-duplication\n");

	str = stringNDuplication(SAMPLE_STRING_SHORT, 3);

	if (strcmp(str, "Hel") != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string n-duplication (%d iteration on %zu bytes)\n", ITER, strlen(SAMPLE_STRING_LONG));

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITER; iteration++) {

		str = stringNDuplication(SAMPLE_STRING_LONG, strlen(SAMPLE_STRING_LONG) / 4);

		free(str);
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %fs.\n", elaps);

	printf("# String concatenation\n");

	str = stringConcatenation(SAMPLE_STRING_SHORT, SAMPLE_STRING_SHORT);

	if (strcmp(str, "Hello world!Hello world!") != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string concatenation (%d iteration on %zu bytes)\n", ITER, strlen(SAMPLE_STRING_LONG));

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITER; iteration++) {

		str = stringConcatenation(SAMPLE_STRING_LONG, SAMPLE_STRING_LONG);

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %fs.\n", elaps);

	printf("# Creating buffer\n");

	buff = createBuffer();

	printf("SUCCESS\n");

	printf("# Buffer to string\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Writing buffer: \"%s\" (%zu bytes)\n", SAMPLE_STRING_SHORT, strlen(SAMPLE_STRING_SHORT));

	writeBuffer(buff, SAMPLE_STRING_SHORT, strlen(SAMPLE_STRING_SHORT));

	if (strncmp(buff->content, "Hello world!", buff->size) != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");	

	printf("# Buffer to string\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Looking buffer: 3 bytes\n");

	str = lookBuffer(buff, 3);

	if (strcmp(str, "Hel") != 0 || buff->size != strlen(SAMPLE_STRING_SHORT))
		ERREXIT("FAILURE");

	free(str);

	printf("SUCCESS\n");	

	printf("# Buffer to string\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Reading buffer: 3 bytes\n");

	str = readBuffer(buff, 3);

	if (strcmp(str, "Hel") != 0 || buff->size != strlen(SAMPLE_STRING_SHORT) - 3)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Buffer to string\n");

	strbuff = bufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Profiling writing/reading buffer (%d iteration on %zu bytes)\n", ITER, strlen(SAMPLE_STRING_LONG));

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITER; iteration++) {
		
		writeBuffer(buff, SAMPLE_STRING_LONG, strlen(SAMPLE_STRING_LONG));

		str = readBuffer(buff, strlen(SAMPLE_STRING_LONG));

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %fs.\n", elaps);

	printf("# Freeing buffer\n");

	freeBuffer(buff);

	printf("SUCCESS\n");

	exit(EXIT_FAILURE);
}
