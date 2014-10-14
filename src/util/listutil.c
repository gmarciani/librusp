#include "listutil.h"

void cleanList(List *list) {
	ListElement *curr = NULL;

	curr = list->head;

	while (curr)
		removeElementFromList(list, curr);
}

void addElementToList(List *list, void *value) {
	ListElement *new = NULL;

	if (!(new = malloc(sizeof(ListElement))))
		ERREXIT("Cannot allocate memory for list element.");

	new->value = value;

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

void removeElementFromList(List *list, ListElement *elem) {
	
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

	free(elem->value);

	free(elem);

	list->size--;
}
