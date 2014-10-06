#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegmentlist.h"

#define NUM_ELEMENTS 10

int main(void) {
	SegmentList *list = NULL;
	Segment sgm;
	char *strlist = NULL;
	int i;

	printf("# Creating segment list #\n");

	list = createSegmentList();

	printf("# Segment list to string #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Adding %d segments to segment list #\n", NUM_ELEMENTS);

	for (i = NUM_ELEMENTS; i > 0; i--) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, i, i, "Hello");

		addSegmentToSegmentList(list, sgm);
	}

	printf("# Segmentlist to string #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Removing segments with minimum and maximum sequence number #\n");

	removeElementFromSegmentList(list, list->head);

	removeElementFromSegmentList(list, list->tail);	

	printf("# Segment list to string #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Cleaning segment list #\n");

	cleanSegmentList(list);

	printf("# Segment list to string #\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Freeing segment list #\n");

	freeSegmentList(list);

	exit(EXIT_SUCCESS);
}
