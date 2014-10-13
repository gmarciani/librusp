#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "../core/rudptsgmbuffer.h"
#include "../util/threadutil.h"

#define NUM_ELEMENTS 10
#define ISN (uint32_t) 0
#define PLD "Hello World!"
#define STATUS 1
#define TIMEOUT (long double) 10.0

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

static TSegmentBuffer *buff;

static void creation(void);

static void stringRepresentation(void);

static void insertion(void);

static void removal(void);

static void deallocation(void);

static void timeoutHandler(union sigval);

int main(void) {

	creation();

	stringRepresentation();

	insertion();

	stringRepresentation();

	removal();

	stringRepresentation();

	sleep(2 * TIMEOUT);

	deallocation();

	exit(EXIT_SUCCESS);
}

static void creation(void) {
	printf("# Creating timeout segment buffer\n");

	buff = createTSegmentBuffer();

	printf("SUCCESS\n");
}

static void stringRepresentation(void) {
	char *strbuff = NULL;

	printf("# Timeout segment buffer to string\n");

	strbuff = tSegmentBufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);
}

static void insertion(void) {
	TSegmentBufferElement *elem = NULL;
	Segment sgm;
	uint32_t seqn = ISN;
	int i;

	printf("# Adding %d segments to timeout segment buffer with ISN %u status %d payload %s and timeout %LF\n", NUM_ELEMENTS, ISN, STATUS, PLD, TIMEOUT);

	for (i = 0; i < NUM_ELEMENTS; i++) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, seqn, i, PLD);					

		elem = addTSegmentBuffer(buff, sgm, STATUS);

		if (!isEqualSegment(sgm, findTSegmentBufferBySequence(buff, sgm.hdr.seqn)->segment))
			ERREXIT("FAILURE");

		if (!isEqualSegment(sgm, findTSegmentBufferByAck(buff, RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds))->segment))
			ERREXIT("FAILURE");

		setTSegmentBufferElementTimeout(elem, TIMEOUT, TIMEOUT, timeoutHandler, elem, sizeof(TSegmentBufferElement));

		seqn = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);
	}

	printf("SUCCESS\n");
}

static void removal(void) {
	printf("# Removing segment with seqn 0\n");

	removeTSegmentBuffer(buff, findTSegmentBufferBySequence(buff, 0));

	if (findTSegmentBufferBySequence(buff, 0) != NULL)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Removing segment acked by ackn 24\n");

	removeTSegmentBuffer(buff, findTSegmentBufferByAck(buff, 24));

	if (findTSegmentBufferByAck(buff, 24) != NULL)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");
}

static void deallocation(void) {
	printf("# Freeing timeout segment buffer\n");

	freeTSegmentBuffer(buff);

	printf("SUCCESS\n");	
}

static void timeoutHandler(union sigval arg) {	
	TSegmentBufferElement *elem = (TSegmentBufferElement *) arg.sival_ptr;
	char *ssgm = NULL;

	if (!elem)
		return;

	ssgm = segmentToString(elem->segment);

	printf("%s\n", ssgm);

	free(ssgm);
}
