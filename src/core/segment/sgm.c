#include "sgm.h"

/* SEGMENT */

const static size_t RUDP_HDRF_SIZE[RUDP_HDRF] = {3, 3, 5, 5, 5, 10, 10};

Segment createSegment(const uint8_t ctrl, const uint16_t urgp, const uint16_t wnds, unsigned long seqn, unsigned long ackn, const char *pld) {
	Segment sgm;
	size_t plds = 0;

	sgm.hdr.vers = RUDP_VERS;

	sgm.hdr.ctrl = ctrl;

	sgm.hdr.urgp = urgp;

	sgm.hdr.wnds = wnds;

	sgm.hdr.seqn = seqn;

	sgm.hdr.ackn = ackn;

	if (pld) {

		plds = (strlen(pld) < RUDP_PLDS) ? strlen(pld) : RUDP_PLDS;

		memcpy(sgm.pld, pld, sizeof(char) * plds);
	}

	sgm.pld[plds] = '\0';

	sgm.hdr.plds = plds;

	return sgm;	
}

Segment deserializeSegment(const char *ssgm) {
	Segment sgm;
	char *hdr = NULL;
	char **hdrfields = NULL;
	size_t plds;
	int i;

	hdr = stringNDuplication(ssgm, RUDP_HDRS);

	hdrfields = splitStringBySection(hdr, RUDP_HDRF_SIZE, RUDP_HDRF);

	sgm.hdr.vers = (uint8_t) atoi(hdrfields[0]);

	sgm.hdr.ctrl = (uint8_t) atoi(hdrfields[1]);

	sgm.hdr.urgp = (uint16_t) atoi(hdrfields[2]);

	sgm.hdr.plds = (uint16_t) atoi(hdrfields[3]);

	sgm.hdr.wnds = (uint16_t) atoi(hdrfields[4]);

	sgm.hdr.seqn = (uint32_t) (strtoul(hdrfields[5], NULL, 10) % RUDP_MAXSEQN);

	sgm.hdr.ackn = (uint32_t) (strtoul(hdrfields[6], NULL, 10) % RUDP_MAXSEQN);

	plds = (sgm.hdr.plds < RUDP_PLDS) ? sgm.hdr.plds : RUDP_PLDS;

	memcpy(sgm.pld, ssgm + RUDP_HDRS, sizeof(char) * plds);

	sgm.pld[plds] = '\0';	

	for (i = 0; i < RUDP_HDRF; i++)
		free(hdrfields[i]);

	free(hdrfields);

	free(hdr);

	return sgm;
}

char *serializeSegment(const Segment sgm) {
	char *ssgm = NULL;

	if (!(ssgm = malloc(sizeof(char) * (RUDP_SGMS + 1)))) 
		ERREXIT("Cannot allocate memory for segment serialization.");

	sprintf(ssgm, "%03u%03u%05u%05u%05u%010u%010u%s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.urgp, sgm.hdr.plds, sgm.hdr.wnds, sgm.hdr.seqn, sgm.hdr.ackn, sgm.pld);

	return ssgm;
}

int isEqualSegment(const Segment sgmone, const Segment sgmtwo) {
	return (sgmone.hdr.vers == sgmtwo.hdr.vers &&
			sgmone.hdr.ctrl == sgmtwo.hdr.ctrl &&
			sgmone.hdr.urgp == sgmtwo.hdr.urgp &&
			sgmone.hdr.plds == sgmtwo.hdr.plds &&
			sgmone.hdr.wnds == sgmtwo.hdr.wnds &&
			sgmone.hdr.seqn == sgmtwo.hdr.seqn &&
			sgmone.hdr.ackn == sgmtwo.hdr.ackn &&
			strcmp(sgmone.pld, sgmtwo.pld) == 0);
}

char *segmentToString(const Segment sgm) {
	char *str = NULL;

	if (!(str = malloc(sizeof(char) * (RUDP_SGMSO + 1))))
		ERREXIT("Error in segment to string allocation.");

	sprintf(str, "vers:%u ctrl:%u urgp:%u plds:%u wnds:%u seqn:%u ackn:%u pld:%s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.urgp, sgm.hdr.plds, sgm.hdr.wnds, sgm.hdr.seqn, sgm.hdr.ackn, sgm.pld);

	return str;
}

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm) {
	char *time;
	char *addr = NULL;
	char *strsgm = NULL;
	int port;

	time = getTime();

	addr = getIp(sndaddr);

	port = getPort(sndaddr);

	strsgm = segmentToString(sgm);

	printf("[<- SGM] %s src: %s:%d %s\n", time, addr, port, strsgm);

	free(time);

	free(addr);

	free(strsgm);
}

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm) {
	char *time;
	char *addr = NULL;
	char *strsgm = NULL;
	int port;

	time = getTime();

	addr = getIp(rcvaddr);

	port = getPort(rcvaddr);

	strsgm = segmentToString(sgm);
	
	printf("[SGM ->] %s dst: %s:%d %s\n", time, addr, port, strsgm);

	free(time);

	free(addr);

	free(strsgm);
}
