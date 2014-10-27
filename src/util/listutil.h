#ifndef LISTUTIL_H_
#define LISTUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "macroutil.h"

#define LIST_INITIALIZER (List) {.size = 0, .nxtid = 0, .head = NULL, .tail = NULL, .rwlock = PTHREAD_RWLOCK_INITIALIZER}

typedef struct ListElement {
	long long id;
	void *value;

	struct ListElement *prev;
	struct ListElement *next;
} ListElement;

typedef struct List {
	long size;
	long long nxtid;

	ListElement *head;
	ListElement *tail;

	pthread_rwlock_t rwlock;
} List;

long long addElementToList(List *list, void *value);

void removeElementFromList(List *list, const long long id);

void *getElementById(List *list, const long long id);

#endif /* LISTUTIL_H_ */
