#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "../../util/stringutil.h"
#include "../../util/macroutil.h"

#define STRING "Hello world!"

#define STRINGSIZE strlen(STRING)

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

	printf("# String duplication...");

	str = stringDuplication(STRING);

	assert((strcmp(str, STRING) == 0));

	printf("OK\n");

	free(str);
}

static void nduplicateString(void) {
	char *str = NULL;

	printf("# String n-duplication...");

	str = stringNDuplication(STRING, 3);

	assert((strcmp(str, "Hel") == 0));

	printf("OK\n");

	free(str);
}

static void concatenateString(void) {
	char *str = NULL;

	printf("# String concatenation...");

	str = stringConcatenation(STRING, STRING);

	assert((strcmp(str, "Hello world!Hello world!") == 0));

	printf("OK\n");

	free(str);
}
