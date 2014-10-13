#ifndef RUDPSGMBUFFER_H_
#define RUDPSGMBUFFER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/times.h>
#include <pthread.h>
#include "rudpsgm.h"
#include "../util/threadutil.h"
#include "../util/timerutil.h"

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

typedef struct SegmentBufferElement {
	Segment segment;
	struct SegmentBufferElement *next;
	struct SegmentBufferElement *prev;
} SegmentBufferElement;

typedef struct SegmentBuffer {
	long size;	
	SegmentBufferElement *head;
	SegmentBufferElement *tail;
	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
	pthread_cond_t *status_cnd;
} SegmentBuffer;

SegmentBuffer *createSegmentBuffer(void);

void freeSegmentBuffer(SegmentBuffer *buff);

SegmentBufferElement *addSegmentBuffer(SegmentBuffer *buff, const Segment sgm);

SegmentBufferElement *findSegmentBufferBySequence(SegmentBuffer *buff, const uint32_t seqn);

SegmentBufferElement *findSegmentBufferByAck(SegmentBuffer *buff, const uint32_t ackn);

void removeSegmentBuffer(SegmentBuffer *buff, SegmentBufferElement *elem);

char *segmentBufferToString(SegmentBuffer *buff);

#endif /* RUDPSGMBUFFER_H_ */
