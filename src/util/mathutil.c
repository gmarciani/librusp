#include "mathutil.h"

uint32_t getRandom32(void) {
	uint32_t random;
	struct timeval timestamp;

	gettimeofday(&timestamp, NULL);

	srand(time(NULL));

	random = (uint32_t) ((rand() * timestamp.tv_sec + rand() * timestamp.tv_usec) % MAX_UINT32);

	return random;
}

uint8_t getRandomBit(const double onprob) {
	uint8_t random;

	random = rand() <  onprob * ((double)RAND_MAX + 1.0);

	return random;
}
