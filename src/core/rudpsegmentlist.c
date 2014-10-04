#include "rudpsegmentlist.h"

SegmentList *createSegmentList(void) {
	SegmentList *list = NULL;

	if (!(list = malloc(sizeof(SegmentList)))) {
		fprintf(stderr, "Cannot allocate memory for segment list.\n");
		exit(EXIT_FAILURE);
	}

	list->size = 0;
	
	list->head = NULL;

	list->tail = NULL;

	return list;
}

void freeSegmentList(SegmentList *list) {
	cleanSegmentList(list);
	free(list);
}

void cleanSegmentList(SegmentList *list) {
	while (list->head)
		removeElementFromSegmentList(list, list->head);
}

void addSegmentToSegmentList(SegmentList *list, const Segment sgm) {
	SegmentListElement *new = NULL;

	if (!(new = malloc(sizeof(SegmentListElement)))) {
		fprintf(stderr, "Cannot allocate memory for new segment list element.\n");
		exit(EXIT_FAILURE);
	}

	if (!(new->segment = malloc(sizeof(Segment)))) {
		fprintf(stderr, "Cannot allocate memory for segment in new segment list element.\n");
		exit(EXIT_FAILURE);
	}

	memcpy(new->segment, &sgm, sizeof(Segment));

	if (list->size == 0) {

		new->prev = NULL;

		new->next = NULL;

		list->head = new;

		list->tail = new;

	} else if (list->head->segment->hdr.seqn > new->segment->hdr.seqn) {			

		new->prev = NULL;

		new->next = list->head;

		list->head->prev = new;

		list->head = new;

	} else if (list->tail->segment->hdr.seqn < new->segment->hdr.seqn) {

		new->prev = list->tail;

		new->next = NULL;

		list->tail->next = new;

		list->tail = new;

	} else {

		SegmentListElement *curr = list->head;

		while (curr) {

			if (curr->segment->hdr.seqn > new->segment->hdr.seqn)
				break;

			curr = curr->next;
		}	

		new->next = curr;

		new->prev = curr->prev;

		curr->prev->next = new;

		curr->prev = new;

	}

	list->size++;
}

void removeElementFromSegmentList(SegmentList *list, SegmentListElement *elem) {

	if (!elem)
		return;

	if ((elem == list->head) && (elem == list->tail)) {

		list->head = NULL;

		list->tail = NULL;			

	} else if (elem == list->head) {

		list->head = elem->next;

		elem->next->prev = NULL;

	} else if (elem == list->tail) {

		list->tail = elem->prev;

		elem->prev->next = NULL;

	} else {

		elem->prev->next = elem->next;

		elem->next->prev = elem->prev;

	}

	free(elem->segment);

	free(elem);

	list->size--;
}

char *segmentListToString(SegmentList *list) {
	SegmentListElement *curr = NULL;
	char *strlist = NULL;
	char *strsgm = NULL;

	if (!(strlist = malloc(sizeof(char) * list->size * (RUDP_MAX_SGM_OUTPUT + 1)))) {
		fprintf(stderr, "Cannot allocate memory for string representation of segment list.\n");
		exit(EXIT_FAILURE);
	}

	strlist[0] = '\0';

	curr = list->head;

	while (curr) {

		strsgm = segmentToString(*(curr->segment));
		
		strcat(strlist, strsgm);

		strcat(strlist, "\n");

		free(strsgm);

		curr = curr->next;
	}

	return strlist;
}