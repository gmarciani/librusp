#ifndef _RUDPSEGMENT_H_
#define _RUDPSEGMENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/addrutil.h"
#include "../util/stringutil.h"

#define RUDP_VERSION 		1

#define RUDP_HDR_FIELDS		7
#define RUDP_MAX_HDR 		(2 * 3) + (3 * 5) + (2 * 10)
#define RUDP_MAX_PLD 		5 	//must be 500
#define RUDP_MAX_SGM		(2 * 3) + (3 * 5) + (2 * 10) + 5	//RUDP_MAX_HDR + RUDP_MAX_PLD

#define RUDP_MAX_SGM_OUTPUT 46 + (2 * 3) + (3 * 5) + (2 * 10) + 5 //output context + RUDP_MAX_SGM

#define RUDP_MAX_SEQN		4294967296

#define RUDP_NULL	0b00000000
#define RUDP_SYN 	0b00000001
#define RUDP_FIN	0b00000010
#define RUDP_RST	0b00000100
#define RUDP_ACK 	0b00001000
#define RUDP_PSH	0b00010000
#define RUDP_URG	0b00100000
#define RUDP_KLV	0b01000000
#define RUDP_ERR 	0b10000000

typedef struct Header {
	uint8_t 	vers;
	uint8_t 	ctrl;
	uint16_t	urgp;
	uint16_t 	plds;
	uint16_t 	wnds;
	uint32_t 	seqn;
	uint32_t 	ackn;
} Header;

typedef struct Segment {
	Header 	hdr;
	char 	pld[RUDP_MAX_PLD + 1];
} Segment;

typedef struct Stream {
	Segment 		*segments;
	unsigned long 	size;
	unsigned long 	len;
} Stream;

/* SEGMENT */

Segment createSegment(const uint8_t ctrl, const uint16_t urgp, const uint16_t wnds, unsigned long seqn, unsigned long ackn, const char *pld);

Segment deserializeSegment(const char *ssgm);

char *serializeSegment(const Segment sgm);

char *segmentToString(const Segment sgm);

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm);

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm);

/* STREAM */

Stream *createStream(const char *msg);

void freeStream(Stream *stream);

char *streamToString(Stream *stream);

#endif /* _RUDPSEGMENT_H_ */
