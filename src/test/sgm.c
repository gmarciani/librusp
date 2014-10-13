#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsgm.h"

#define MSG "Hello World! I'm happy! Stay hungry, stay foolish, folks!"

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

static Segment sgm;

static void creation(void);

static void stringRepresentation(void);

static void serializationDeserialization(void);

static void sequenceMatching(void);

int main(void) {

	creation();

	stringRepresentation();

	serializationDeserialization();

	sequenceMatching();

	exit(EXIT_SUCCESS);	
}

static void creation(void) {
	printf("# Creating SYNACK segments (PLDS=%d) with urgp=5 wnds=324, seqn=51, ackn=22, pld=%s\n", RUDP_PLDS, MSG);

	sgm = createSegment(RUDP_SYN | RUDP_ACK, 5, 324, 51, 22, MSG);

	printf("SUCCESS\n");	
}

static void stringRepresentation(void) {
	char *strsgm = NULL;

	printf("# Segment to string\n");

	strsgm = segmentToString(sgm);

	printf("%s\n", strsgm);

	free(strsgm);
}

static void serializationDeserialization(void) {
	Segment sgmtwo;
	char *ssgm = NULL;

	printf("# Serializing/Deserializing segment\n");

	ssgm = serializeSegment(sgm);

	sgmtwo = deserializeSegment(ssgm);

	free(ssgm);

	if (!isEqualSegment(sgm, sgmtwo))
		ERREXIT("FAILURE");

	printf("SUCCESS\n");
}

static void sequenceMatching(void) {
	uint32_t wndb, wnde, seqn;

	printf("# Matching sequence number inside window\n");

	wndb = 0;
	wnde = 2097152;
	seqn = 80;

	while (seqn != 79) {
		
		if (matchSequenceAgainstWindow(wndb, wnde, seqn) != 0) {
			printf("wndb:%u wnde:%u seqn:%u\n", wndb, wnde, seqn);
			ERREXIT("MATCHING FAILURE: sequence should be inside window.");
		}

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("SUCCESS\n");

	printf("# Matching sequence number before window\n");

	wndb = 500;
	wnde = 2097652;
	seqn = 80;

	while (seqn != 79) {		
		
		if (matchSequenceAgainstWindow(wndb, wnde, seqn) != -1) {
			printf("wndb:%u wnde:%u seqn:%u\n", wndb, wnde, seqn);
			ERREXIT("FAILURE: sequence should be before window.");
		}

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("SUCCESS\n");

	printf("# Matching sequence number after window\n");

	wndb = 500;
	wnde = 2097652;
	seqn = 2107152;

	while (seqn != 2107151) {
		
		if (matchSequenceAgainstWindow(wndb, wnde, seqn) != 1) {
			printf("wndb:%u wnde:%u seqn:%u\n", wndb, wnde, seqn);	
			ERREXIT("MATCHING FAILURE: sequence should be after window.");
		}

		wndb = RUDP_NXTSEQN(wndb, 1);
		wnde = RUDP_NXTSEQN(wnde, 1);
		seqn = RUDP_NXTSEQN(seqn, 1);
	}

	printf("SUCCESS\n");
}
