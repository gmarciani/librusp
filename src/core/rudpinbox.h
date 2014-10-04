#ifndef _RUDPINBOX_H_
#define _RUDPINBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rudpsegment.h"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

typedef struct InboxElement {
	uint8_t status;
	Segment *segment;
	struct InboxElement *prev;
	struct InboxElement *next;
} InboxElement;

typedef struct Inbox {
	InboxElement *head;
	InboxElement *tail;
	InboxElement *wndbase;
	InboxElement *wndend;	
	unsigned long size;
	unsigned long wndsize;	
	unsigned long awndsize;
	unsigned long nextseqno;
} Inbox;

Inbox *createInbox(const uint32_t isn, const uint32_t wnds);

void freeInbox(Inbox *inbox);

Segment readInboxSegment(Inbox *inbox);

char *readInboxBuffer(Inbox *inbox, const size_t size);

void submitSegmentToInbox(Inbox *inbox, const Segment sgm);

char *inboxToString(Inbox *inbox);

#endif /* _RUDPINBOX_H_ */
