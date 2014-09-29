#ifndef _RUDPINBOX_H_
#define _RUDPINBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rudpsegment.h"

typedef struct InboxElement {
	unsigned int status;
	Segment *segment;
	struct InboxElement *prev;
	struct InboxElement *next;
} InboxElement;

typedef struct SegmentInbox {
	InboxElement *head;
	InboxElement *tail;
	InboxElement *wndbase;
	InboxElement *wndend;	
	unsigned long size;
	unsigned long wndsize;	
	unsigned long awndsize;
	unsigned long nextseqno;
} SegmentInbox;

SegmentInbox *createInbox(const uint32_t isn, const uint32_t wnds);

void freeInbox(SegmentInbox *inbox);

Segment readInboxSegment(SegmentInbox *inbox);

char *readInboxBuffer(SegmentInbox *inbox, const size_t size);

void submitSegmentToInbox(SegmentInbox *inbox, const Segment sgm);

char *inboxToString(const SegmentInbox inbox);

#endif /* _RUDPINBOX_H_ */
