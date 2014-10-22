#include <stdio.h>
#include <stdlib.h>
#include "../../util/timeutil.h"
#include "../../util/threadutil.h"

#define ITERATIONS 1000000000000

static void profileEquality(void);

static void profileInequality(void);

static void profileDisequality(void);

int main(void) {

	profileEquality();

	profileInequality();

	profileDisequality();

	exit(EXIT_SUCCESS);
}

static void profileEquality(void) {
	long i;
	int value = 0;

	for (i = 0; i < ITERATIONS; i++) {
		if (value == -1)
			continue;
	}
}

static void profileInequality(void) {
	long i;
	int value = 0;

	for (i = 0; i < ITERATIONS; i++) {
		if (value != 0)
			continue;
	}
}

static void profileDisequality(void) {
	long i;
	int value = 0;

	for (i = 0; i < ITERATIONS; i++) {
		if (value > 0)
			continue;
	}
}
