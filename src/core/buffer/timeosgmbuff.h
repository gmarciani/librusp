#ifndef TIMEOSGMBUFF_H_
#define TIMEOSGMBUFF_H_

#include "../segment/sgm.h"
#include "../../util/threadutil.h"
#include "../../util/timerutil.h"
#include "../../util/macroutil.h"

#define RUDP_SGM_NACKED 0
#define RUDP_SGM_YACKED 1;

/* TIMEOUT SEGMENT BUFFER STRUCTURES */

typedef struct TSegmentBufferElement {
	short status;
	Segment segment;

	int retrans;
	long double offset;

	struct TSegmentBufferElement *next;
	struct TSegmentBufferElement *prev;
} TSegmentBufferElement;

typedef struct TSegmentBuffer {
	long size;

	TSegmentBufferElement *head;
	TSegmentBufferElement *tail;

	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
	pthread_cond_t *status_cnd;
} TSegmentBuffer;

/* TIMEOUT SEGMENT BUFFER CREATION/DISTRUCTION */

TSegmentBuffer *createTSegmentBuffer(void);

void freeTSegmentBuffer(TSegmentBuffer *buff);

/* TIMEOUT SEGMENT BUFFER INSERTION/REMOVAL */

TSegmentBufferElement *addTSegmentBuffer(TSegmentBuffer *buff, const Segment sgm);

void removeTSegmentBuffer(TSegmentBuffer *buff, TSegmentBufferElement *elem);

/* TIMEOUT SEGMENT BUFFER SEARCH */

TSegmentBufferElement *findTSegmentBufferBySequence(TSegmentBuffer *buff, const uint32_t seqn);

TSegmentBufferElement *findTSegmentBufferByAck(TSegmentBuffer *buff, const uint32_t ackn);

/* TIMEOUT SEGMENT BUFFER ELEMENT STATUS */

void setTSegmentBufferElementStatus(TSegmentBufferElement *elem, const short status);

short getTSegmentBufferElementStatus(TSegmentBufferElement *elem);

/* TIMEOUT SEGMENT BUFFER ELEMENT TIMEOUT */

void attachTSegmentBufferTimeout(TSegmentBufferElement *elem, void (*handler) (union sigval), void *arg, size_t argsize);

void startTSegmentBufferElementTimeout(TSegmentBufferElement *elem, const long double value, const long double ivalue);

void stopTSegmentBufferElementTimeout(TSegmentBufferElement *elem);

/* TIMEOUT SEGMENT BUFFER REPRESENTATION */

char *tSegmentBufferToString(TSegmentBuffer *buff);

#endif /* TIMEOSGMBUFF_H_ */
