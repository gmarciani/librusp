#include "mathutil.h"

unsigned long getRandomUL(void) {
	unsigned long random;
	struct timeval timestamp;

	gettimeofday(&timestamp, NULL);

	random = ((unsigned long)(drand48() * timestamp.tv_sec) + (unsigned long)(drand48() * timestamp.tv_usec)) % ULONG_MAX;

	return random;
}

unsigned short getRandomBit(const double onprob) {
	unsigned short random;

	random = drand48() < onprob;

	return random;
}

unsigned long getMD5(const char *input) {
	unsigned char hash128[MD5_DIGEST_LENGTH];
	char strhash32[MD5_DIGEST_LENGTH + 1];
	unsigned long hash;
	int i;
 
  	MD5((const unsigned char *)input, strlen(input), hash128);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(strhash32 + i, "%d", hash128[i]);

	strhash32[i] = '\0';

	hash = strtoull(strhash32, NULL, 10) % ULONG_MAX;
 
	return hash;
}
