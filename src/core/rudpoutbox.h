#ifndef _RUDPOUTBOX_H_
#define _RUDPOUTBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "rudpsegment.h"
#include "../util/threadutil.h"

#define RUDP_UNACKED 	0
#define RUDP_ACKED		1

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

typedef struct OutboxElement {
	uint8_t status;
	Segment *segment;
	struct OutboxElement *prev;
	struct OutboxElement *next;
} OutboxElement;

typedef struct Outbox {
	OutboxElement *head;
	OutboxElement *tail;
	OutboxElement *wndb;
	OutboxElement *wnde;	
	uint32_t size;
	uint32_t wnds;	
	uint32_t awnds;
	uint32_t nextseqn;
	pthread_mutex_t *outbox_mtx;
	pthread_cond_t *outbox_cnd;
} Outbox;

Outbox *createOutbox(const uint32_t isn, const uint32_t wnds);

void freeOutbox(Outbox *outbox);

void writeOutboxUserBuffer(Outbox *outbox, const char *msg, const size_t size);

void submitSegmentToOutbox(Outbox *outbox, const Segment sgm);

void submitAckToOutbox(Outbox *outbox, const uint32_t ackn);

Segment *getRetransmittableSegments(Outbox *outbox, uint32_t *retransno);

char *outboxToString(Outbox *outbox);

#endif /* _RUDPOUTBOX_H_ */
