#ifndef LISTUTIL_H_
#define LISTUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "macroutil.h"

#define LIST_INITIALIZER (List) {.size = 0, .head = NULL, .tail = NULL}

typedef struct ListElement {
	void *value;
	struct ListElement *prev;
	struct ListElement *next;
} ListElement;

typedef struct List {
	uint32_t size;
	ListElement *head;
	ListElement *tail;
} List;

void cleanList(List *list);

void addElementToList(List *list, void *value);

void removeElementFromList(List *list, ListElement *elem);

#endif /* LISTUTIL_H_ */
