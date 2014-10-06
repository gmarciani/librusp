#include <stdlib.h>
#include <stdio.h>
#include "../core/rudpsegmentlist.h"

#define NUM_ELEMENTS 10

int main(void) {
	SegmentList *list = NULL;
	Segment sgm;
	char *strlist = NULL;
	int i;

	printf("# Creating segment list\n");

	list = createSegmentList();

	printf("# Segment list to string\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Adding %d segments to segment list\n", NUM_ELEMENTS);

	for (i = NUM_ELEMENTS; i > 0; i--) {
	
		sgm = createSegment(RUDP_ACK, 0, 0, i, i, "Hello");

		addSegmentToSegmentList(list, sgm);
	}

	printf("Added to segment list\n");

	printf("# Segmentlist to string:\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Removing head and tail:\n");

	removeElementFromSegmentList(list, list->head);

	printf("Head removed\n");

	removeElementFromSegmentList(list, list->tail);	

	printf("Tail removed\n");

	printf("# Segment list to string:\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Cleaning segment list\n");

	cleanSegmentList(list);

	printf("Segment list cleaned\n");

	printf("# Segment list to string:\n");

	strlist = segmentListToString(list);

	printf("%s\n", strlist);

	free(strlist);

	printf("# Freeing segment list\n");

	freeSegmentList(list);

	printf("Segment list freed\n");

	exit(EXIT_SUCCESS);
}
