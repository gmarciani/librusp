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

	random = rand() <  onprob * ((double) RAND_MAX + 1.0);

	return random;
}

uint32_t getMD5(const char *input) {
	unsigned char hash128[MD5_DIGEST_LENGTH];
	char strhash32[MD5_DIGEST_LENGTH + 1];
	uint32_t hash32;
	int i;
 
  	MD5((const unsigned char *)input, strlen(input), hash128);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(strhash32 + i, "%d", hash128[i]);

	strhash32[i] = '\0';

	hash32 = (uint32_t) strtoull(strhash32, NULL, 10) % MAX_UINT32;
 
	return hash32;
}
