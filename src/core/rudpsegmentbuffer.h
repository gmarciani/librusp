#ifndef _RUDPSEGMENTBUFFER_H_
#define _RUDPSEGMENTBUFFER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "rudpsegment.h"
#include "../util/timerutil.h"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

/* NORMAL SEGMENT BUFFER */

typedef struct SegmentBufferElement {
	Segment segment;
	struct SegmentBufferElement *next;
	struct SegmentBufferElement *prev;
} SegmentBufferElement;

typedef struct SegmentBuffer {
	uint32_t size;	
	SegmentBufferElement *head;
	SegmentBufferElement *tail;
} SegmentBuffer;

SegmentBuffer *createSegmentBuffer(void);

void freeSegmentBuffer(SegmentBuffer *buff);

SegmentBufferElement *addSegmentBuffer(SegmentBuffer *buff, const Segment sgm);

SegmentBufferElement *findSegmentBuffer(SegmentBuffer *buff, const uint32_t seqn);

void removeSegmentBuffer(SegmentBuffer *buff, SegmentBufferElement *elem);

char *segmentBufferToString(SegmentBuffer *buff);

/* TIMEOUT SEGMENT BUFFER */

typedef struct TSegmentBufferElement {
	uint8_t status;
	timer_t timer;
	void *timerarg;
	Segment segment;
	struct TSegmentBufferElement *next;
	struct TSegmentBufferElement *prev;
} TSegmentBufferElement;

typedef struct TSegmentBuffer {
	uint32_t size;	
	TSegmentBufferElement *head;
	TSegmentBufferElement *tail;
} TSegmentBuffer;

TSegmentBuffer *createTSegmentBuffer(void);

void freeTSegmentBuffer(TSegmentBuffer *buff);

TSegmentBufferElement *addTSegmentBuffer(TSegmentBuffer *buff, const Segment sgm, const uint8_t status, const uint64_t nanos, void (*handler) (union sigval), void *arg, size_t argsize);

TSegmentBufferElement *findTSegmentBuffer(TSegmentBuffer *buff, const uint32_t seqn);

void removeTSegmentBuffer(TSegmentBuffer *buff, TSegmentBufferElement *elem);

char *tSegmentBufferToString(TSegmentBuffer *buff);

#endif /* _RUDPSEGMENTBUFFER_H_ */
