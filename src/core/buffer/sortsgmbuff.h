#ifndef SORTSGMBUFF_H_
#define SORTSGMBUFF_H_

#include "../segment/sgm.h"
#include "../../util/threadutil.h"
#include "../../util/timerutil.h"
#include "../../util/macroutil.h"

/* SORTED SEGMENT BUFFER STRUCTURES */

typedef struct SegmentBufferElement {
	short status;
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

/* SORTED SEGMENT BUFFER CREATION/DISTRUCTION */

SegmentBuffer *createSegmentBuffer(void);

void freeSegmentBuffer(SegmentBuffer *buff);

/* SORTED SEGMENT BUFFER INSERTION/REMOVAL */

SegmentBufferElement *addSegmentBuffer(SegmentBuffer *buff, const Segment sgm);

void removeSegmentBuffer(SegmentBuffer *buff, SegmentBufferElement *elem);

/* SORTED SEGMENT BUFFER SEARCH */

SegmentBufferElement *findSegmentBufferBySequence(SegmentBuffer *buff, const uint32_t seqn);

SegmentBufferElement *findSegmentBufferByAck(SegmentBuffer *buff, const uint32_t ackn);

/* SORTED SEGMENT BUFFER REPRESENTATION */

char *segmentBufferToString(SegmentBuffer *buff);

#endif /* SORTSGMBUFF_H_ */
