#include "seqn.h"

int matchSequenceAgainstWindow(const uint32_t wndb, const uint32_t wnde, const uint32_t seqn) {
	if ((wndb < wnde && seqn >= wndb && seqn < wnde) ||
		(wndb > wnde && (seqn >= wndb || seqn < wnde)))
		return 0;
	else if (wndb - seqn <= (RUSP_MAXSEQN / 2))
		return -1;
	else
		return 1;
}

uint32_t getRandomSequence(const struct sockaddr_in laddr, const struct sockaddr_in paddr) {
	char strladdr[ADDRIPV4_STR], strpaddr[ADDRIPV4_STR];
	struct timespec time;
	uint32_t isn;

	addressToString(laddr, strladdr);

	addressToString(paddr, strladdr);

	clock_gettime(CLOCK_REALTIME, &time);

	isn = fmod((time.tv_sec * 1000000.0 + time.tv_nsec / 1000.0) / 4.0 + getMD5(strladdr) + getMD5(strpaddr) + getRandomUL(), RUSP_MAXSEQN);
	
	return isn;	
}
