#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../core/segment/sgm.h"
#include "../../util/macroutil.h"

#define MSG "Hello World!"
#define ADDR "192.168.0.1"
#define PORT 8888

static Segment sgm;

static void creation(void);

static void stringRepresentation(void);

static void serializationDeserialization(void);

static void printInOutSegment(void);

static void sequenceComparation(void);

static void sequenceMatching(void);

int main(void) {

	creation();

	stringRepresentation();

	serializationDeserialization();

	printInOutSegment();

	sequenceComparation();

	sequenceMatching();

	exit(EXIT_SUCCESS);	
}

static void creation(void) {
	printf("# Creating SYNACK segments (RUDP_PLDS=%d) with plds:12 seqn:51 ackn:22 pld:%s...", RUSP_PLDS, MSG);

	sgm = createSegment(RUSP_SYN | RUSP_SACK, strlen(MSG), 51, 22, MSG);

	printf("OK\n");
}

static void stringRepresentation(void) {
	char strsgm[RUSP_SGM_STR];

	printf("# Segment to string...");

	segmentToString(sgm, strsgm);

	assert(strcmp(strsgm, "ctrl:9 plds:12 seqn:51 ackn:22 Hello World!") == 0);

	printf("OK\n");
}

static void serializationDeserialization(void) {
	Segment sgmtwo;
	char ssgm[RUSP_SGMS];

	printf("# Serializing/Deserializing segment...");

	serializeSegment(sgm, ssgm);

	deserializeSegment(ssgm, &sgmtwo);

	assert(isEqualSegment(sgm, sgmtwo));

	printf("OK\n");
}

static void printInOutSegment(void) {
	struct sockaddr_in addr = createAddress(ADDR, PORT);

	printf("# Printing In/Out segments...\n");

	printInSegment(addr, sgm);

	printOutSegment(addr, sgm);

	printf("OK\n");
}

static void sequenceComparation(void) {
	uint32_t seqnone, seqntwo;

	printf("# Comparing sequence numbers less than...");

	seqnone = 10;
	seqntwo = 20;

	while (seqnone != 9) {

		assert(RUSP_LTSEQN(seqnone, seqntwo));

		seqnone = RUSP_NXTSEQN(seqnone, 1);

		seqntwo = RUSP_NXTSEQN(seqntwo, 1);
	}

	printf("OK\n");
}

static void sequenceMatching(void) {
	uint32_t wndb, wnde, seqn;

	printf("# Matching sequence number inside window...");

	wndb = 0;
	wnde = 2097152;
	seqn = 80;

	while (seqn != 79) {
		
		assert(matchSequenceAgainstWindow(wndb, wnde, seqn) == 0);

		wndb = RUSP_NXTSEQN(wndb, 1);
		wnde = RUSP_NXTSEQN(wnde, 1);
		seqn = RUSP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");

	printf("# Matching sequence number before window...");

	wndb = 500;
	wnde = 2097652;
	seqn = 80;

	while (seqn != 79) {		
		
		assert(matchSequenceAgainstWindow(wndb, wnde, seqn) == -1);

		wndb = RUSP_NXTSEQN(wndb, 1);
		wnde = RUSP_NXTSEQN(wnde, 1);
		seqn = RUSP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");

	printf("# Matching sequence number after window...");

	wndb = 500;
	wnde = 2097652;
	seqn = 2107152;

	while (seqn != 2107151) {
		
		assert(matchSequenceAgainstWindow(wndb, wnde, seqn) == 1);

		wndb = RUSP_NXTSEQN(wndb, 1);
		wnde = RUSP_NXTSEQN(wnde, 1);
		seqn = RUSP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");
}
