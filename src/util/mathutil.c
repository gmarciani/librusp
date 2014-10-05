#include "mathutil.h"

uint32_t getRandom32(void) {
	uint32_t random;
	struct timeval time;

	gettimeofday(&time, NULL);

	random = (uint32_t) ((1000 * time.tv_sec + time.tv_usec) % 4294967295);

	return random;
}

uint8_t getRandomBit(const double onprob) {
	uint8_t random;

	random = (rand() % RAND_MAX) > onprob;

	return random;
}
