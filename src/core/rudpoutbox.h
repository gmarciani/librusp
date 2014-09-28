#ifndef _RUDPOUTBOX_H_
#define _RUDPOUTBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rudpsegment.h"

#define _RUDP_UNACKED 	0
#define _RUDP_ACKED		1

typedef struct OutboxElement {
	unsigned int status;
	Segment *segment;
	struct OutboxElement *prev;
	struct OutboxElement *next;
} OutboxElement;

typedef struct SegmentOutbox {
	OutboxElement *head;
	OutboxElement *tail;
	OutboxElement *wndbase;
	OutboxElement *wndend;	
	unsigned long size;
	unsigned long wndsize;	
	unsigned long awndsize;
	unsigned long nextseqno;
} SegmentOutbox;

SegmentOutbox createOutbox(const unsigned long isn, const unsigned long wndsize);

void freeOutbox(SegmentOutbox *outbox);

void submitSegmentToOutbox(SegmentOutbox *outbox, const Segment sgm);

void submitAckToOutbox(SegmentOutbox *outbox, const unsigned long ackno);

void _slideOutboxWindow(SegmentOutbox *outbox);

void _removeOutboxElement(SegmentOutbox *outbox, OutboxElement *elem);

char *outboxToString(const SegmentOutbox outbox);

#endif /* _RUDPOUTBOX_H_ */
