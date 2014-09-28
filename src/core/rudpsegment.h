#ifndef _RUDPSEGMENT_H_
#define _RUDPSEGMENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/addrutil.h"
#include "../util/stringutil.h"

#define _RUDP_VERSION 1

#define _RUDP_HDR_FIELDS	5
#define _RUDP_MAX_HDR 		29
#define _RUDP_MAX_PLD 		5
#define _RUDP_MAX_SGM		34	

#define _RUDP_MAX_SGM_OUTPUT 79

#define _RUDP_SYN 	0
#define _RUDP_FIN	1
#define _RUDP_ACK 	2
#define _RUDP_DAT 	3
#define _RUDP_EOS	4
#define _RUDP_ERR 	5

typedef struct Header {
	unsigned short vers; // 2 byte
	unsigned short ctrl; // 2 byte
	unsigned short plds; // 5 byte
	unsigned long seqno; // 10 byte
	unsigned long ackno; // 10 byte
} Header;

typedef struct Segment {
	Header hdr;
	char pld[_RUDP_MAX_PLD + 1];
} Segment;

typedef struct Stream {
	Segment *segments;
	unsigned long size;
	unsigned long len;
} Stream;

/* SEGMENT */

Segment createSegment(const unsigned short ctrl, unsigned long seqno, unsigned long ackno, const char *pld);

Segment deserializeSegment(const char *ssgm);

char *serializeSegment(const Segment sgm);

char *segmentToString(const Segment sgm);

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm);

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm);

/* STREAM */

Stream createStream(const char *msg);

void freeStream(Stream *stream);

#endif /* _RUDPSEGMENT_H_ */
