#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../../util/mathutil.h"
#include "../../util/macroutil.h"

#define STRING "Hy folks! I'm Jack. Let's us enjoy MD5!"
#define NUMRAND 50000
#define CONSTRAINT 0.0001L
#define NONPROB 0.00L
#define LONPROB 0.03L
#define MONPROB 0.30L
#define HONPROB 1.00L

void generateRandomUL(void);

void generateRandomBit(const long double onprob);

static void generateMD5(void);

int main(void) {	

	generateRandomUL();

	generateRandomBit(NONPROB);

	generateRandomBit(LONPROB);

	generateRandomBit(MONPROB);

	generateRandomBit(HONPROB);

	generateMD5();

	exit(EXIT_SUCCESS);
}

void generateRandomUL(void) {
	unsigned long randomUL[NUMRAND], checked[NUMRAND];
	unsigned long repetitions;
	unsigned long i, j, k, l;

	printf("# Generating %d random unsigned long, expecting max %LF%% repetitions...", NUMRAND, CONSTRAINT);

	for (i = 0; i < NUMRAND; i++)
		randomUL[i] = getRandomUL();
	
	repetitions = 0;

	l = 0;

	for (i = 0; i < NUMRAND; i++) {

		for (k = 0; k < l; k++) {
			
			if (randomUL[i] == checked[k])
				break;
		}

		if (k < l)
			continue;

		checked[l] = randomUL[i];
		
		l++;
		
		for (j = 0; j < NUMRAND; j++) {
			
			if (j == i)
				continue;

			if (randomUL[j] == randomUL[i])
				repetitions++;
		}
	}

	assert(repetitions <= (CONSTRAINT * 100 * NUMRAND));

	printf("OK\n");
}

void generateRandomBit(const long double onprob) {
	int randombit[NUMRAND];
	long double constraint;
	unsigned long repetitions, i;

	printf("%d", NUMRAND);

	constraint = (onprob == 0.0 || onprob == 1.0) ? 0.0 : CONSTRAINT;

	printf("# Generating %d random bits, with %LF%% ON probability, expecting max %LF%% error...", NUMRAND, onprob, constraint);

	for (i = 0; i < NUMRAND; i++)
		randombit[i] = getRandomBit(NONPROB);

	repetitions = 0;

	for (i = 0; i < NUMRAND; i++) {

		if (randombit[i] == 1)
			repetitions++;
	}

	assert(repetitions >= ((NONPROB - constraint) * 100 * NUMRAND) &&
			repetitions <= ((NONPROB + constraint) * 100 * NUMRAND));

	printf("OK\n");
}

static void generateMD5(void) {
	unsigned long hash;

	printf("# Generating MD5 hashing for: %s...", STRING);

	hash = getMD5(STRING);

	printf("%lu...", hash);

	printf("OK\n");
}
