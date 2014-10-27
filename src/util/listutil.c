#include "listutil.h"

long long addElementToList(List *list, void *value) {
	ListElement *new = NULL;
	long long id;

	if (!(new = malloc(sizeof(ListElement))))
		ERREXIT("Cannot allocate memory for list element.");

	new->value = value;

	if (pthread_rwlock_wrlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	new->id = list->nxtid;

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

	list->nxtid++;

	id = new->id;

	if (pthread_rwlock_unlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return id;
}

void removeElementFromList(List *list, const long long id) {
	ListElement *elem = NULL;

	if (pthread_rwlock_wrlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	elem = list->head;

	while (elem) {

		if (elem->id == id)
			break;

		elem = elem->next;
	}

	if (!elem) {

		if (pthread_rwlock_unlock(&(list->rwlock)) > 0)
			ERREXIT("Cannot release read-write lock.");

		return;
	}

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

	list->size--;

	if (pthread_rwlock_unlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	free(elem->value);

	free(elem);
}

void *getElementById(List *list, const long long id) {
	ListElement *elem = NULL;
	void *value = NULL;

	if (pthread_rwlock_rdlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	elem = list->head;

	while (elem) {

		if (elem->id == id)
			break;

		elem = elem->next;
	}

	if (!elem) {

		if (pthread_rwlock_unlock(&(list->rwlock)) > 0)
			ERREXIT("Cannot release read-write lock.");

		return NULL;
	}

	value = elem->value;

	if (pthread_rwlock_unlock(&(list->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return value;
}
