#ifndef SGM_H_
#define SGM_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "seqn.h"
#include "../../util/addrutil.h"
#include "../../util/stringutil.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

// Version
#define RUDP_VERS 1

// Control Bits
#define RUDP_NUL 0b00000000
#define RUDP_SYN 0b00000001
#define RUDP_FIN 0b00000010
#define RUDP_RST 0b00000100
#define RUDP_ACK 0b00001000
#define RUDP_PSH 0b00010000
#define RUDP_URG 0b00100000
#define RUDP_KLV 0b01000000
#define RUDP_ERR 0b10000000

// Payload
#define RUDP_PLDS 500

//Segment Serialization
#define RUDP_HDRF 7
#define RUDP_HDRS 41
#define RUDP_SGMS (RUDP_HDRS + RUDP_PLDS)

//Segment String
#define RUDP_HDR_STR (RUDP_HDRS + 42)
#define RUDP_SGM_STR (RUDP_HDR_STR + RUDP_PLDS)

typedef struct Header {
	uint8_t vers;
	uint8_t ctrl;
	uint16_t urgp;
	uint16_t plds;
	uint16_t wnds;
	uint32_t seqn;
	uint32_t ackn;
} Header;

typedef struct Segment {
	Header hdr;
	char pld[RUDP_PLDS];
} Segment;

/* SEGMENT */

Segment createSegment(const uint8_t ctrl, const uint16_t urgp, const uint16_t plds, const uint16_t wnds, const uint32_t seqn, const uint32_t ackn, const char *pld);

size_t serializeSegment(const Segment sgm, char *ssgm);

void deserializeSegment(const char *ssgm, Segment *sgm);

int isEqualSegment(const Segment sgmone, const Segment sgmtwo);

void segmentToString(const Segment sgm, char *str);

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm);

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm);

#endif /* SGM_H_ */
