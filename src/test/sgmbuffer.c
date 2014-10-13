#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include "../core/rudpsgmbuffer.h"
#include "../util/threadutil.h"

#define NUM_ELEMENTS 10
#define ISN (uint32_t) 0
#define PLD "Hello World!"

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

static SegmentBuffer *buff;

static void creation(void);

static void stringRepresentation(void);

static void insertion(void);

static void removal(void);

static void deallocation(void);

int main(void) {

	creation();

	stringRepresentation();

	insertion();	

	stringRepresentation();

	removal();

	stringRepresentation();

	deallocation();

	exit(EXIT_SUCCESS);
}

static void creation(void) {
	printf("# Creating segment buffer\n");

	buff = createSegmentBuffer();

	printf("SUCCESS\n");
}

static void stringRepresentation(void) {
	char *strbuff = NULL;

	printf("# Segment buffer to string\n");

	strbuff = segmentBufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);
}

static void insertion(void) {
	Segment sgm;
	uint32_t seqn = ISN;
	int i;

	printf("# Adding %d segments to segment buffer, with ISN %u and payload %s\n", NUM_ELEMENTS, ISN, PLD);

	for (i = 0; i < NUM_ELEMENTS; i++) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, seqn, i, PLD);

		addSegmentBuffer(buff, sgm);

		if (!isEqualSegment(sgm, findSegmentBufferBySequence(buff, sgm.hdr.seqn)->segment))
			ERREXIT("FAILURE");

		if (!isEqualSegment(sgm, findSegmentBufferByAck(buff, RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds))->segment))
			ERREXIT("FAILURE");

		seqn = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);
	}

	printf("SUCCESS\n");
}

static void removal(void) {
	printf("# Removing segment with seqn 0\n");

	removeSegmentBuffer(buff, findSegmentBufferBySequence(buff, 0));

	if (findSegmentBufferBySequence(buff, 0) != NULL)
		ERREXIT("FAILURE");

	printf("# Removing segment acked by ackn 24\n");

	removeSegmentBuffer(buff, findSegmentBufferByAck(buff, 24));

	if (findSegmentBufferByAck(buff, 24) != NULL)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");
}

static void deallocation(void) {
	printf("# Freeing segment buffer\n");

	freeSegmentBuffer(buff);

	printf("SUCCESS\n");
}
