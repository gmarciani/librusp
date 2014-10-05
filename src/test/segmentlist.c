#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegmentlist.h"

#define NUM_ELEMENTS 10

int main(void) {
	SegmentList *list = NULL;
	SegmentListElement *curr = NULL;
	Segment sgm;
	char *strlist = NULL;
	int i;

	printf("# Creating segment list #\n");

	list = createSegmentList();

	printf("# Adding %d segments to segment list #\n", NUM_ELEMENTS);

	for (i = NUM_ELEMENTS; i >= 0; i--) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, i, 0, NULL);

		addSegmentToSegmentList(list, sgm);

	}

	printf("# Getting string representation of segment list #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Removing segment with sequence number 6 from segment list #\n");

	curr = list->head;

	while (curr) {
		
		if (curr->segment->hdr.seqn == 6)
			break;

		curr = curr->next;
	}

	removeElementFromSegmentList(list, curr);

	printf("# Getting string representation of segment list #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Cleaning segment list #\n");

	cleanSegmentList(list);

	printf("# Getting string representation of segment list #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Freeing segment list #\n");

	freeSegmentList(list);

	exit(EXIT_SUCCESS);
}
