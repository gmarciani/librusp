#include "rudpsegmentlist.h"

SegmentList *createSegmentList(void) {
	SegmentList *list = NULL;

	if (!(list = malloc(sizeof(SegmentList))))
		ERREXIT("Cannot allocate memory for segment list.");

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

	if (!(new = malloc(sizeof(SegmentListElement))))
		ERREXIT("Cannot allocate memory for new segment list element.");

	new->segment = sgm;

	if (list->size == 0) {

		new->prev = NULL;

		new->next = NULL;

		list->head = new;

		list->tail = new;

	} else {

		new->prev = list->tail;

		new->next = NULL;

		list->tail->next = new;

		list->tail = new;
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

	free(elem);

	list->size--;
}

char *segmentListToString(SegmentList *list) {
	SegmentListElement *curr = NULL;
	char *strlist = NULL;
	char *strsgms = NULL;
	char *strsgm = NULL;

	if (!(strsgms = malloc(sizeof(char) * (list->size * (RUDP_SGMSO + 1) + 1))))
		ERREXIT("Cannot allocate memory for string representation of segment list.");

	strsgms[0] = '\0';

	curr = list->head;

	while (curr) {

		strsgm = segmentToString(curr->segment);
		
		strcat(strsgms, strsgm);

		if (curr->next)
			strcat(strsgms, "\n");

		free(strsgm);

		curr = curr->next;
	}

	if (!(strlist = malloc(sizeof(char) * (25 + strlen(strsgms) + 1))))
		ERREXIT("Cannot allocate memory for string representation of segment list.");

	sprintf(strlist, "size:%u content:%s%s", list->size, ((list->size > 0) ? "\n" : ""), strsgms);

	free(strsgms);
	
	return strlist;
}
