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

// Control Bits
#define RUSP_NUL  0b00000000
#define RUSP_SYN  0b00000001
#define RUSP_FIN  0b00000010
#define RUSP_RST  0b00000100
#define RUSP_SACK 0b00001000
#define RUSP_CACK 0b00010000
#define RUSP_PSH  0b00100000
#define RUSP_KLV  0b01000000
#define RUSP_ERR  0b10000000

// Payload
#define RUSP_PLDS 1000

//Segment Serialization
#define RUSP_HDRF 4
#define RUSP_HDRS 28
#define RUSP_SGMS (RUSP_HDRS + RUSP_PLDS)

//Segment String
#define RUSP_HDR_STR (RUSP_HDRS + 24 + 1)
#define RUSP_SGM_STR (RUSP_HDR_STR + RUSP_PLDS)

typedef struct Header {
	uint8_t ctrl;
	uint16_t plds;
	uint32_t seqn;
	uint32_t ackn;
} Header;

typedef struct Segment {
	Header hdr;
	char pld[RUSP_PLDS];
} Segment;

/* SEGMENT */

Segment createSegment(const uint8_t ctrl, const uint16_t plds, const uint32_t seqn, const uint32_t ackn, const char *pld);

size_t serializeSegment(const Segment sgm, char *ssgm);

void deserializeSegment(const char *ssgm, Segment *sgm);

int isEqualSegment(const Segment sgmone, const Segment sgmtwo);

void segmentToString(const Segment sgm, char *str);

void printInSegment(const struct sockaddr_in sndaddr, const Segment sgm);

void printOutSegment(const struct sockaddr_in rcvaddr, const Segment sgm);

#endif /* SGM_H_ */
