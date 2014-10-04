#ifndef _RUDPSEGMENTLIST_H_
#define _RUDPSEGMENTLIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "rudpsegment.h"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

typedef struct SegmentListElement {
	Segment *segment;
	struct SegmentListElement *next;
	struct SegmentListElement *prev;
} SegmentListElement;

typedef struct SegmentList {
	uint32_t size;
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
