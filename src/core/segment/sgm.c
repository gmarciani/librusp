#include "sgm.h"

/* SEGMENT */

Segment createSegment(const uint8_t ctrl, const uint16_t plds, const uint32_t seqn, const uint32_t ackn, const char *pld) {
	Segment sgm;

	sgm.hdr.ctrl = ctrl;

	sgm.hdr.seqn = seqn;

	sgm.hdr.ackn = ackn;

	if (pld) {

		sgm.hdr.plds = MIN(plds, RUSP_PLDS);

		memcpy(sgm.pld, pld, sizeof(char) * sgm.hdr.plds);

	} else {

		sgm.hdr.plds = 0;
	}

	return sgm;	
}

size_t serializeSegment(const Segment sgm, char *ssgm) {
	size_t ssgmsize;

	sprintf(ssgm, "%03u%05u%010u%010u%.*s", sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqn, sgm.hdr.ackn, (int)sgm.hdr.plds, sgm.pld);

	ssgmsize = strlen(ssgm);

	return ssgmsize;
}

void deserializeSegment(const char *ssgm, Segment *sgm) {
	char hdrf[RUSP_HDRF][11];

	memcpy(hdrf[0], ssgm, sizeof(char) * 3);
	hdrf[0][3] = '\0';

	memcpy(hdrf[1], ssgm + 3, sizeof(char) * 5);
	hdrf[1][5] = '\0';

	memcpy(hdrf[2], ssgm + 8, sizeof(char) * 10);
	hdrf[2][10] = '\0';

	memcpy(hdrf[3], ssgm + 18, sizeof(char) * 10);
	hdrf[3][10] = '\0';

	sgm->hdr.ctrl = (uint8_t) atoi(hdrf[0]);

	sgm->hdr.plds = (uint16_t) MIN(atoi(hdrf[1]), RUSP_PLDS);

	sgm->hdr.seqn = (uint32_t) (strtoul(hdrf[2], NULL, 10) % RUSP_MAXSEQN);

	sgm->hdr.ackn = (uint32_t) (strtoul(hdrf[3], NULL, 10) % RUSP_MAXSEQN);

	memcpy(sgm->pld, ssgm + RUSP_HDRS, sizeof(char) * sgm->hdr.plds);
}

int isEqualSegment(const Segment sgmone, const Segment sgmtwo) {
	return (sgmone.hdr.ctrl == sgmtwo.hdr.ctrl &&
			sgmone.hdr.plds == sgmtwo.hdr.plds &&
			sgmone.hdr.seqn == sgmtwo.hdr.seqn &&
			sgmone.hdr.ackn == sgmtwo.hdr.ackn &&
			memcmp(sgmone.pld, sgmtwo.pld, sgmone.hdr.plds) == 0);
}

void segmentToString(const Segment sgm, char *str) {
	sprintf(str, "ctrl:%u plds:%u seqn:%u ackn:%u %.*s", sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqn, sgm.hdr.ackn, (int)sgm.hdr.plds, sgm.pld);
}

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm) {
	char strsgm[RUSP_SGM_STR], time[TIME_STR], addr[ADDRIPV4_STR];

	getTime(time);

	addressToString(sndaddr, addr);

	segmentToString(sgm, strsgm);

	printf("[<- SGM] %s src: %s %s\n", time, addr, strsgm);
}

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm) {
	char strsgm[RUSP_SGM_STR], time[TIME_STR], addr[ADDRIPV4_STR];

	getTime(time);

	addressToString(rcvaddr, addr);

	segmentToString(sgm, strsgm);
	
	printf("[SGM ->] %s dst: %s %s\n", time, addr, strsgm);
}
