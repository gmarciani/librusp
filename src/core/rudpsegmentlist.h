#ifndef _RUDPSEGMENTLIST_H_
#define _RUDPSEGMENTLIST_H_

#include "rudpsegment.h"

typedef struct SegmentListElement {
	Segment *segment;
	struct SegmentListElement *next;
	struct SegmentListElement *prev;
} SegmentListElement;

typedef struct SegmentList {
	unsigned long size;
	SegmentListElement *head;
	SegmentListElement *tail;
} SegmentList;

SegmentList *createSegmentList(void);

void freeSegmentList(SegmentList *list);

void cleanSegmentList(SegmentList *list);

void addSegmentToSegmentList(SegmentList *list, const Segment sgm);

void removeElementFromSegmentList(SegmentList *list, SegmentListElement *elem);

char *segmentListToString(SegmentList *list);

#endif /* _RUDPSEGMENTLIST_H_ */
