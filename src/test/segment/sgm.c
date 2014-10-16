#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../core/segment/sgm.h"
#include "../../util/macroutil.h"

#define MSG "Hello World!"

static Segment sgm;

static void creation(void);

static void stringRepresentation(void);

static void serializationDeserialization(void);

static void sequenceComparation(void);

static void sequenceMatching(void);

int main(void) {

	creation();

	stringRepresentation();

	serializationDeserialization();

	sequenceComparation();

	sequenceMatching();

	exit(EXIT_SUCCESS);	
}

static void creation(void) {
	printf("# Creating SYNACK segments (PLDS=%d) with urgp=5 wnds=324, seqn=51, ackn=22, pld=%s...", RUDP_PLDS, MSG);

	sgm = createSegment(RUDP_SYN | RUDP_ACK, 5, 324, 51, 22, MSG);

	printf("OK\n");
}

static void stringRepresentation(void) {
	char *strsgm = NULL;

	printf("# Segment to string...");

	strsgm = segmentToString(sgm);

	printf("%s\n", strsgm);

	free(strsgm);
}

static void serializationDeserialization(void) {
	Segment sgmtwo;
	char *ssgm = NULL;

	printf("# Serializing/Deserializing segment...");

	ssgm = serializeSegment(sgm);

	sgmtwo = deserializeSegment(ssgm);

	free(ssgm);

	assert(isEqualSegment(sgm, sgmtwo));

	printf("OK\n");
}

static void sequenceComparation(void) {
	uint32_t seqnone, seqntwo;

	printf("# Comparing less than...");

	seqnone = 10;
	seqntwo = 20;

	while (seqnone != 9) {

		assert(RUDP_LTSEQN(seqnone, seqntwo));

		seqnone = RUDP_NXTSEQN(seqnone, 1);

		seqntwo = RUDP_NXTSEQN(seqntwo, 1);
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

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");

	printf("# Matching sequence number before window...");

	wndb = 500;
	wnde = 2097652;
	seqn = 80;

	while (seqn != 79) {		
		
		assert(matchSequenceAgainstWindow(wndb, wnde, seqn) == -1);

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");

	printf("# Matching sequence number after window...");

	wndb = 500;
	wnde = 2097652;
	seqn = 2107152;

	while (seqn != 2107151) {
		
		assert(matchSequenceAgainstWindow(wndb, wnde, seqn) == 1);

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("OK\n");
}
