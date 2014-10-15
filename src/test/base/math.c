#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../../util/mathutil.h"
#include "../../util/macroutil.h"

#define STRING "Hy folks! I'm Jack. Let's us enjoy MD5!"
#define NUMRAND 500000
#define NONPROB 0.00
#define LONPROB 0.03
#define MONPROB 0.30
#define HONPROB 1.00

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

	printf("# Generating %ld random unsigned long\n", (unsigned long)NUMRAND);

	for (i = 0; i < NUMRAND; i++)
		randomUL[i] = getRandomUL();

	printf("SUCCESS\n");

	printf("# Analyzing generated random unsigned long\n");
	
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

	printf("Repetition Percentage: %LF%%\n", (long double)(repetitions / NUMRAND));
}

void generateRandomBit(const long double onprob) {
	unsigned short randombit[NUMRAND];
	long double onperc;
	unsigned long repetitions, i;

	printf("# Generating %ld random bits (uniform ON: %LF%%)\n", (unsigned long)NUMRAND, onprob);

	for (i = 0; i < NUMRAND; i++)
		randombit[i] = getRandomBit(onprob);

	printf("SUCCESS\n");

	printf("# Analyzing generated random bits\n");

	repetitions = 0;

	onperc = 0.0;

	for (i = 0; i < NUMRAND; i++) {

		if (randombit[i] == 1)
			repetitions++;
	}

	onperc = ((long double) repetitions * 100.0) / (long double) NUMRAND;

	printf("ON Percentage: %LF%%\n", onperc);
}

static void generateMD5(void) {
	unsigned long hash;

	printf("# Generating MD5 hashing for: %s\n", STRING);

	hash = getMD5(STRING);

	printf("%lu\n", hash);

	printf("SUCCESS\n");
}
