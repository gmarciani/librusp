#include "rudpsegment.h"

/* SEGMENT */

const static size_t RUDP_HDRF_SIZE[RUDP_HDRF] = {3, 3, 5, 5, 5, 10, 10};

Segment createSegment(const uint8_t ctrl, const uint16_t urgp, const uint16_t wnds, unsigned long seqn, unsigned long ackn, const char *pld) {
	Segment sgm;
	size_t pldsize = 0;
	int i;

	sgm.hdr.vers = RUDP_VERS;

	sgm.hdr.ctrl = ctrl;

	sgm.hdr.urgp = urgp;

	sgm.hdr.wnds = wnds;

	sgm.hdr.seqn = seqn;

	sgm.hdr.ackn = ackn;

	if (pld) {

		pldsize = (strlen(pld) < RUDP_PLDS) ? strlen(pld) : RUDP_PLDS;

		for (i = 0; i < pldsize; i++)
			sgm.pld[i] = pld[i];

	}

	sgm.pld[pldsize] = '\0';

	sgm.hdr.plds = pldsize;

	return sgm;	
}

Segment deserializeSegment(const char *ssgm) {
	Segment sgm;
	char *hdr = NULL;
	char **hdrfields = NULL;
	size_t pldsize;
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

	pldsize = (sgm.hdr.plds < RUDP_PLDS) ? sgm.hdr.plds : RUDP_PLDS;

	for (i = 0; i < pldsize; i++)
		sgm.pld[i] = *(ssgm + RUDP_HDRS + i);

	sgm.pld[i] = '\0';	

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

/* STREAM */

Stream *createStream(const char *msg) {
	Stream *stream = NULL;
	char **chunks = NULL;
	size_t chunksize;
	int numchunks = 0;
	int i, j;

	if (!(stream = malloc(sizeof(Stream))))
		ERREXIT("Cannot allocate memory for stream of segments.");

	chunks = splitStringBySize(msg, RUDP_PLDS, &numchunks);

	if (!(stream->segments = malloc(sizeof(Segment) * numchunks)))
		ERREXIT("Cannot allocate memory for stream of segments.");

	memset(stream->segments, 0, sizeof(Segment) * numchunks);

	stream->size = 0;

	stream->len = 0;	

	for (i = 0; i < numchunks; i++) {

		stream->segments[i].hdr.vers = RUDP_VERS;

		stream->segments[i].hdr.ctrl = RUDP_NUL;

		stream->segments[i].hdr.urgp = 0;

		stream->segments[i].hdr.wnds = 0;

		stream->segments[i].hdr.seqn = 0;

		stream->segments[i].hdr.ackn = 0;

		chunksize = strlen(chunks[i]);

		for (j = 0; j < chunksize; j++) {

			stream->segments[i].hdr.plds++;

			stream->segments[i].pld[j] = chunks[i][j];

			stream->len += 1;
		}

		stream->segments[i].pld[j] = '\0';

		stream->size += 1;

	}

	stream->segments[i - 1].hdr.ctrl |= RUDP_PSH;

	for (i = 0; i < numchunks; i++) 
		free(chunks[i]);

	free(chunks);

	return stream;
}

void freeStream(Stream *stream) {
	cleanStream(stream);

	free(stream);
}

void cleanStream(Stream *stream) {
	if (stream->segments)
		free(stream->segments);

	stream->segments = NULL;	
	
	stream->size = 0;

	stream->len = 0;
}

char *streamToString(Stream *stream) {
	char *strstream = NULL;
	char *strsgm = NULL;
	int i;

	if (!(strstream = malloc(sizeof(char) * (38 + stream->size * (RUDP_SGMSO + 1) + 1))))
		ERREXIT("Cannot allocate memory for stream to string.");

	sprintf(strstream, "Stream size:%u len:%u\n", stream->size, stream->len);

	for (i = 0; i < stream->size; i++) {

		strsgm = segmentToString(stream->segments[i]);

		strcat(strstream, strsgm);

		strcat(strstream, "\n");

		free(strsgm);

	}

	return strstream;
}
