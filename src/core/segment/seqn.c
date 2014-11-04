#include "seqn.h"

int matchSequenceAgainstWindow(const uint32_t wndb, const uint32_t wnde, const uint32_t seqn) {
	if ((wndb < wnde && seqn >= wndb && seqn < wnde) ||
		(wndb > wnde && (seqn >= wndb || seqn < wnde)))
		return 0;
	else if (wndb - seqn <= (RUDP_MAXSEQN / 2))
		return -1;
	else
		return 1;
}

uint32_t getRandomSequence(const struct sockaddr_in laddr, const struct sockaddr_in paddr) {
	char strladdr[ADDRIPV4_STR], strpaddr[ADDRIPV4_STR];
	uint32_t isn;

	addressToString(laddr, strladdr);

	addressToString(paddr, strladdr);

	isn = (((getMD5(strladdr) + getMD5(strpaddr)) % clock()) + getRandomUL()) % RUDP_MAXSEQN;
	
	return isn;	
}
