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

SegmentInbox createInbox(const unsigned long isn, const unsigned long wndsize);

void freeInbox(SegmentInbox *inbox);

void submitSegmentToInbox(SegmentInbox *inbox, const Segment sgm);

Segment readInboxSegment(SegmentInbox *inbox);

char *readInboxBuffer(SegmentInbox *inbox, const size_t size);

void _slideInboxWindow(SegmentInbox *inbox);

void _removeInboxElement(SegmentInbox *inbox, InboxElement *elem);

char *inboxToString(const SegmentInbox inbox);

#endif /* _RUDPINBOX_H_ */
