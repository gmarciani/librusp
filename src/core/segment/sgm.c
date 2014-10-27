#include "sgm.h"

/* SEGMENT */

Segment createSegment(const uint8_t ctrl, const uint16_t urgp, const uint16_t plds, const uint16_t wnds, const uint32_t seqn, const uint32_t ackn, const char *pld) {
	Segment sgm;

	sgm.hdr.vers = RUDP_VERS;

	sgm.hdr.ctrl = ctrl;

	sgm.hdr.urgp = urgp;

	sgm.hdr.wnds = wnds;

	sgm.hdr.seqn = seqn;

	sgm.hdr.ackn = ackn;

	if (pld) {

		sgm.hdr.plds = MIN(plds, RUDP_PLDS);

		memcpy(sgm.pld, pld, sizeof(char) * sgm.hdr.plds);

	} else {

		sgm.hdr.plds = 0;
	}

	return sgm;	
}

size_t serializeSegment(const Segment sgm, char *ssgm) {
	size_t ssgmsize;

	sprintf(ssgm, "%03u%03u%05u%05u%05u%010u%010u%.*s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.urgp, sgm.hdr.plds, sgm.hdr.wnds, sgm.hdr.seqn, sgm.hdr.ackn, (int)sgm.hdr.plds, sgm.pld);

	ssgmsize = strlen(ssgm);

	return ssgmsize;
}

void deserializeSegment(const char *ssgm, Segment *sgm) {
	char hdr[RUDP_HDRS + 1];
	char hdrf[RUDP_HDRF][11];

	memcpy(hdr, ssgm, RUDP_HDRS);
	hdr[RUDP_HDRS] = '\0';

	memcpy(hdrf[0], ssgm, sizeof(char) * 3);
	hdrf[0][3] = '\0';

	memcpy(hdrf[1], ssgm + 3, sizeof(char) * 3);
	hdrf[1][3] = '\0';

	memcpy(hdrf[2], ssgm + 6, sizeof(char) * 5);
	hdrf[2][5] = '\0';

	memcpy(hdrf[3], ssgm + 11, sizeof(char) * 5);
	hdrf[3][5] = '\0';

	memcpy(hdrf[4], ssgm + 16, sizeof(char) * 5);
	hdrf[4][5] = '\0';

	memcpy(hdrf[5], ssgm + 21, sizeof(char) * 10);
	hdrf[5][10] = '\0';

	memcpy(hdrf[6], ssgm + 31, sizeof(char) * 10);
	hdrf[6][10] = '\0';

	sgm->hdr.vers = (uint8_t) atoi(hdrf[0]);

	sgm->hdr.ctrl = (uint8_t) atoi(hdrf[1]);

	sgm->hdr.urgp = (uint16_t) atoi(hdrf[2]);

	sgm->hdr.plds = (uint16_t) MIN(atoi(hdrf[3]), RUDP_PLDS);

	sgm->hdr.wnds = (uint16_t) atoi(hdrf[4]);

	sgm->hdr.seqn = (uint32_t) (strtoul(hdrf[5], NULL, 10) % RUDP_MAXSEQN);

	sgm->hdr.ackn = (uint32_t) (strtoul(hdrf[6], NULL, 10) % RUDP_MAXSEQN);

	memcpy(sgm->pld, ssgm + RUDP_HDRS, sizeof(char) * sgm->hdr.plds);
}

int isEqualSegment(const Segment sgmone, const Segment sgmtwo) {
	return (sgmone.hdr.vers == sgmtwo.hdr.vers &&
			sgmone.hdr.ctrl == sgmtwo.hdr.ctrl &&
			sgmone.hdr.urgp == sgmtwo.hdr.urgp &&
			sgmone.hdr.plds == sgmtwo.hdr.plds &&
			sgmone.hdr.wnds == sgmtwo.hdr.wnds &&
			sgmone.hdr.seqn == sgmtwo.hdr.seqn &&
			sgmone.hdr.ackn == sgmtwo.hdr.ackn &&
			memcmp(sgmone.pld, sgmtwo.pld, sgmone.hdr.plds) == 0);
}

void segmentToString(const Segment sgm, char *str) {
	sprintf(str, "vers:%u ctrl:%u urgp:%u plds:%u wnds:%u seqn:%u ackn:%u %.*s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.urgp, sgm.hdr.plds, sgm.hdr.wnds, sgm.hdr.seqn, sgm.hdr.ackn, (int)sgm.hdr.plds, sgm.pld);
}

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm) {
	char strsgm[RUDP_SGM_STR], time[TIME_STR], addr[ADDRIPV4_STR];

	getTime(time);

	addressToString(sndaddr, addr);

	segmentToString(sgm, strsgm);

	printf("[<- SGM] %s src: %s %s\n", time, addr, strsgm);
}

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm) {
	char strsgm[RUDP_SGM_STR], time[TIME_STR], addr[ADDRIPV4_STR];

	getTime(time);

	addressToString(rcvaddr, addr);

	segmentToString(sgm, strsgm);
	
	printf("[SGM ->] %s dst: %s %s\n", time, addr, strsgm);
}
