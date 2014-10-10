#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "../util/threadutil.h"
#include "../core/rudpsegmentbuffer.h"

#define NUM_ELEMENTS 10
#define TIMEOUT 4000000000

static int VAL = 0;
static pthread_mutex_t MTX = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t CND = PTHREAD_COND_INITIALIZER;

static void timeoutHandler(union sigval);

typedef struct TimeoutObject {
	Segment segment;
	uint32_t *lastackn;
	int sock;
} TimeoutObject;

static void timeoutHandler(union sigval arg) {
	TimeoutObject *timobj = (TimeoutObject *) arg.sival_ptr;
	Segment sgm = timobj->segment;	
	char *ssgm = NULL;	

	lockMutex(&MTX);

	sgm.hdr.ackn = *(timobj->lastackn);

	ssgm = segmentToString(sgm);

	printf("%s\n", ssgm);

	free(ssgm);

	VAL++;
	signalConditionVariable(&CND);

	unlockMutex(&MTX);
}

int main(void) {
	SegmentBuffer *buff = NULL;
	TSegmentBuffer *tbuff = NULL;
	Segment sgm;
	char *strbuff, *strtbuff = NULL;
	int i;

	printf("# Creating segment buffer\n");

	buff = createSegmentBuffer();

	printf("SUCCESS\n");

	printf("# Segment buffer to string\n");

	strbuff = segmentBufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Adding %d segments to segment buffer\n", NUM_ELEMENTS);

	for (i = NUM_ELEMENTS; i > 0; i--) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, i, i, "Hello World!");

		addSegmentBuffer(buff, sgm);

		if (!isEqualSegment(sgm, findSegmentBuffer(buff, sgm.hdr.seqn)->segment))
			ERREXIT("FAILURE");
	}

	printf("SUCCESS\n");

	printf("# Segment buffer to string\n");

	strbuff = segmentBufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Removing segment with seqn 5\n");

	removeSegmentBuffer(buff, findSegmentBuffer(buff, 5));

	if (findSegmentBuffer(buff, 5) != NULL)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Segment buffer to string\n");

	strbuff = segmentBufferToString(buff);

	printf("%s\n", strbuff);

	free(strbuff);

	printf("# Freeing segment buffer\n");

	freeSegmentBuffer(buff);

	printf("SUCCESS\n");

	printf("# Creating timeout segment buffer\n");

	tbuff = createTSegmentBuffer();

	printf("SUCCESS\n");

	printf("# Timeout segment buffer to string\n");

	strtbuff = tSegmentBufferToString(tbuff);

	printf("%s\n", strtbuff);

	free(strtbuff);

	printf("# Adding %d segments to timeout segment buffer with timeout: %lu nanos\n", NUM_ELEMENTS, TIMEOUT);

	uint32_t lastackn = 100;
	uint32_t nextseqn = 0;

	TimeoutObject timobj;
	timobj.lastackn = &lastackn;

	for (i = 0; i < NUM_ELEMENTS; i++) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, nextseqn, lastackn, "Hello World!");

		nextseqn = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);		

		timobj.segment = sgm;		

		addTSegmentBuffer(tbuff, sgm, 18, TIMEOUT, timeoutHandler, &timobj, sizeof(TimeoutObject));

		if (!isEqualSegment(sgm, findTSegmentBuffer(tbuff, sgm.hdr.seqn)->segment))
			ERREXIT("FAILURE");
	}

	lastackn = 500;

	printf("SUCCESS\n");

	printf("# Timeout segment buffer to string\n");

	strtbuff = tSegmentBufferToString(tbuff);

	printf("%s\n", strtbuff);

	free(strtbuff);

	lockMutex(&MTX);
	while (VAL < 10)
		waitConditionVariable(&CND, &MTX);

	printf("# Removing segment with seqn 0\n");

	removeTSegmentBuffer(tbuff, findTSegmentBuffer(tbuff, 0));

	if (findTSegmentBuffer(tbuff, 0) != NULL)
		ERREXIT("FAILURE");

	printf("SUCCESS\n");

	printf("# Timeout segment buffer to string\n");

	strtbuff = tSegmentBufferToString(tbuff);

	printf("%s\n", strtbuff);

	free(strtbuff);

	lastackn = 700;

	VAL = 0;

	unlockMutex(&MTX);

	lockMutex(&MTX);
	while (VAL < 9)
		waitConditionVariable(&CND, &MTX);

	unlockMutex(&MTX);

	printf("# Freeing timeout segment buffer\n");

	freeTSegmentBuffer(tbuff);

	printf("SUCCESS\n");	

	exit(EXIT_SUCCESS);
}
