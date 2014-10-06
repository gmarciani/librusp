#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../util/mathutil.h"

#define NUMRAND (long) 10000

static uint32_t RANDOM32[NUMRAND];

static uint8_t RANDOMBIT[NUMRAND];

static long MAXREPETITIONS;

static long TOTREPETITIONS;

static double ONPERC;

void generateRandom32(void);

void generateRandomBit(const double onprob);

void analyzeRandom32(void);

void analyzeRandomBit(void);

int main(void) {

	printf("# Generating %ld random 32-bit words.\n", NUMRAND);

	generateRandom32();

	printf("# Analyzing generated random 32-bit words:\n");
	
	analyzeRandom32();	

	printf("Tot.Repetitions:%ld Max.Repetitions: %ld\n", TOTREPETITIONS, MAXREPETITIONS);

	printf("# Generating %ld random bits (uniform ON: %f)\n", NUMRAND, 0.0);

	generateRandomBit(0.0);

	printf("# Analyzing generated random bits:\n");
	
	analyzeRandomBit();	

	printf("ON Percentage:%f\n", ONPERC);

	printf("# Generating %ld random bits (uniform ON: %f)\n", NUMRAND, 1.0);

	generateRandomBit(1.0);

	printf("# Analyzing generated random bits:\n");
	
	analyzeRandomBit();	

	printf("ON Percentage:%f\n", ONPERC);

	printf("# Generating %ld random bits (uniform ON: %f)\n", NUMRAND, 0.3);

	generateRandomBit(0.3);

	printf("# Analyzing generated random bits:\n");
	
	analyzeRandomBit();	

	printf("ON Percentage:%f\n", ONPERC);

	exit(EXIT_SUCCESS);
}

void generateRandom32(void) {
	long i;

	for (i = 0; i < NUMRAND; i++)
		RANDOM32[i] = getRandom32();
}

void generateRandomBit(const double onprob) {
	long i;

	for (i = 0; i < NUMRAND; i++)
		RANDOMBIT[i] = getRandomBit(onprob);
}

void analyzeRandom32(void) {
	long repetitions, i, j;
	
	TOTREPETITIONS = 0;
	
	MAXREPETITIONS = 0;

	for (i = 0; i < NUMRAND; i++) {

		repetitions = 0;
		
		for (j = 0; j < NUMRAND; j++) {
			
			if (j == i)
				continue;

			if (RANDOM32[j] == RANDOM32[i])
				repetitions++;
		}

		if (repetitions > MAXREPETITIONS)
			MAXREPETITIONS = repetitions;

		TOTREPETITIONS += repetitions;
	}
}

void analyzeRandomBit(void) {
	long repetitions, i;

	repetitions = 0;

	ONPERC = 0.0;

	for (i = 0; i < NUMRAND; i++) {

		if (RANDOMBIT[i] == 1)
			repetitions++;
	}

	ONPERC = ((double) repetitions * 100.0) / (double) NUMRAND;
}
