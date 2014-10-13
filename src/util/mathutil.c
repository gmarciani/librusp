#include "mathutil.h"

uint32_t getRandom32(void) {
	uint32_t random;
	struct timeval timestamp;

	gettimeofday(&timestamp, NULL);

	random = ((uint32_t) (drand48() * timestamp.tv_sec) + (uint32_t) (drand48() * timestamp.tv_usec)) % MAX_UINT32;

	return random;
}

uint8_t getRandomBit(const double onprob) {
	uint8_t random;

	random = drand48() < onprob;

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
