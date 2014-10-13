#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../util/stringutil.h"

#define ITERATIONS 10000000

#define STRING "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define STRING2 "Hello world!"

#define STRINGSIZE strlen(STRING)

#define STRINGSIZE2 strlen(STRING2)

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

static void duplicateString(void);

static void nduplicateString(void);

static void concatenateString(void);

int main(void) {
	
	duplicateString();

	nduplicateString();

	concatenateString();

	exit(EXIT_FAILURE);
}

static void duplicateString(void) {
	char *str = NULL;
	struct timespec start, end;
	long double elaps;	
	unsigned long iteration;

	printf("# String duplication\n");

	str = stringDuplication(STRING2);

	if (strcmp(str, STRING2) != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string duplication (%d iteration on %zu bytes)\n", ITERATIONS, STRINGSIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITERATIONS; iteration++) {

		str = stringDuplication(STRING);

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = ((long double)(end.tv_sec - start.tv_sec) + (long double)(end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %LF seconds.\n", elaps);
}

static void nduplicateString(void) {
	char *str = NULL;
	struct timespec start, end;
	long double elaps;	
	unsigned long iteration;

	printf("# String n-duplication\n");

	str = stringNDuplication(STRING2, 3);

	if (strcmp(str, "Hel") != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string n-duplication (%d iteration on %zu bytes)\n", ITERATIONS, STRINGSIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITERATIONS; iteration++) {

		str = stringNDuplication(STRING, STRINGSIZE / 2);

		free(str);
	}
	
	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = ((long double)(end.tv_sec - start.tv_sec) + (long double)(end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %LF seconds.\n", elaps);
}

static void concatenateString(void) {
	char *str = NULL;
	struct timespec start, end;
	long double elaps;	
	unsigned long iteration;

	printf("# String concatenation\n");

	str = stringConcatenation(STRING2, STRING2);

	if (strcmp(str, "Hello world!Hello world!") != 0)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	free(str);

	printf("# Profiling string concatenation (%d iteration on %zu bytes)\n", ITERATIONS, STRINGSIZE);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < ITERATIONS; iteration++) {

		str = stringConcatenation(STRING, STRING);

		free(str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = ((long double)(end.tv_sec - start.tv_sec) + (long double)(end.tv_nsec - start.tv_nsec) / 1000000000.0);

	printf("elapsed: %LF seconds.\n", elaps);
}
