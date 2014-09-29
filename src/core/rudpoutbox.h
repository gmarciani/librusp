#ifndef _RUDPOUTBOX_H_
#define _RUDPOUTBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "rudpsegment.h"

#define RUDP_UNACKED 	0
#define RUDP_ACKED		1

typedef struct OutboxElement {
	uint8_t status;
	Segment *segment;
	struct OutboxElement *prev;
	struct OutboxElement *next;
} OutboxElement;

typedef struct SegmentOutbox {
	OutboxElement *head;
	OutboxElement *tail;
	OutboxElement *wndb;
	OutboxElement *wnde;	
	uint32_t size;
	uint32_t wnds;	
	uint32_t awnds;
	uint32_t nextseqn;
} SegmentOutbox;

SegmentOutbox *createOutbox(const uint32_t isn, const uint32_t wnds);

void freeOutbox(SegmentOutbox *outbox);

void submitSegmentToOutbox(SegmentOutbox *outbox, const Segment sgm);

void submitAckToOutbox(SegmentOutbox *outbox, const uint32_t ackn);

Segment *getRetransmittableSegments(SegmentOutbox *outbox, uint32_t *retransno);

char *outboxToString(SegmentOutbox *outbox);

#endif /* _RUDPOUTBOX_H_ */
