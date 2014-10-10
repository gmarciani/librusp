#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegment.h"

#define MSG "Hello World! I'm happy! Stay hungry, stay foolish, folks!"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

int main(void) {
	Segment sgmone, sgmtwo;
	char *ssgm = NULL;
	uint32_t wndb, wnde, seqn;

	printf("# Creating SYNACK segments (PLDS=%d) with urgp=5 wnds=324, seqn=51, ackn=22, pld=%s\n", RUDP_PLDS, MSG);

	sgmone = createSegment(RUDP_SYN | RUDP_ACK, 5, 324, 51, 22, MSG);

	printf("SUCCESS\n");	

	printf("# Segment to string\n");

	ssgm = segmentToString(sgmone);

	printf("%s\n", ssgm);

	free(ssgm);

	printf("# Serializing/Deserializing segment\n");

	ssgm = serializeSegment(sgmone);

	sgmtwo = deserializeSegment(ssgm);

	free(ssgm);

	if (!isEqualSegment(sgmone, sgmtwo))
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

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

	exit(EXIT_SUCCESS);	
}
