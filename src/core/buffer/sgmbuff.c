#include "sgmbuff.h"

/* SEGMENT BUFFER CREATION/DISTRUCTION */

void initializeSgmBuff(SgmBuff *buff) {

	pthread_rwlock_init(&(buff->rwlock), NULL);

	pthread_mutex_init(&(buff->mtx), NULL);

	pthread_cond_init(&(buff->insert_cnd), NULL);

	pthread_cond_init(&(buff->remove_cnd), NULL);

	pthread_cond_init(&(buff->status_cnd), NULL);

	buff->size = 0;

	buff->head = NULL;

	buff->tail = NULL;
}

void destroySgmBuff(SgmBuff *buff) {

	while (buff->head)
		removeSgmBuff(buff, buff->head);

	if (pthread_cond_destroy(&(buff->insert_cnd)) > 0)
		ERREXIT("Cannot destroy segment-buffer insert condition variable.");

	if  (pthread_cond_destroy(&(buff->remove_cnd)) > 0)
		ERREXIT("Cannot destroy segment-buffer remove condition variable.");

	if  (pthread_cond_destroy(&(buff->status_cnd)) > 0)
		ERREXIT("Cannot destroy segment-buffer status condition variable.");

	if (pthread_rwlock_destroy(&(buff->rwlock)) > 0)
		ERREXIT("Cannot destroy segment-buffer read-write lock.");

	if (pthread_mutex_destroy(&(buff->mtx)) > 0)
		ERREXIT("Cannot destroy segment-buffer mutex.");

	buff->size = 0;

	buff->head = NULL;

	buff->tail = NULL;
}

/* SEGMENT BUFFER INSERTION/REMOVAL */

SgmBuffElem *addSgmBuff(SgmBuff *buff, const Segment sgm, const int status) {
	SgmBuffElem *new = NULL;

	if (!(new = malloc(sizeof(SgmBuffElem))))
		ERREXIT("Cannot allocate memory for new segment buffer element.");

	new->status = status;

	new->retrans = 0;

	clock_gettime(CLOCK_MONOTONIC, &(new->time));

	new->delay = 0.0;

	new->segment = sgm;

	if (pthread_rwlock_init(&(new->rwlock), NULL) > 0)
		ERREXIT("Cannot initialize read-write lock.");

	if (pthread_rwlock_wrlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff write-lock for add.");

	if (buff->size == 0) {

		new->prev = NULL;

		new->next = NULL;

		buff->head = new;

		buff->tail = new;

	} else {

		new->prev = buff->tail;

		new->next = NULL;

		buff->tail->next = new;

		buff->tail = new;
	} 

	buff->size++;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for add.");

	if (pthread_cond_broadcast(&(buff->insert_cnd)) > 0)
		ERREXIT("Cannot broadcast condition variable.");

	return new;
}

void removeSgmBuff(SgmBuff *buff, SgmBuffElem *elem) {

	if (!elem)
		return;

	if (pthread_rwlock_wrlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff write-lock for remove.");

	if ((elem == buff->head) && (elem == buff->tail)) {

		buff->head = NULL;

		buff->tail = NULL;

	} else if (elem == buff->head) {

		buff->head = elem->next;

		elem->next->prev = NULL;

	} else if (elem == buff->tail) {

		buff->tail = elem->prev;

		elem->prev->next = NULL;

	} else {

		elem->prev->next = elem->next;

		elem->next->prev = elem->prev;

	}

	buff->size--;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for remove.");

	if (pthread_cond_broadcast(&(buff->remove_cnd)) > 0)
		ERREXIT("Cannot broadcast condition variable.");

	if (pthread_rwlock_destroy(&(elem->rwlock)) > 0)
		ERREXIT("Cannot destroy read-write lock.");

	free(elem);
}

long getSgmBuffSize(SgmBuff *buff) {
	long size;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for getting size.");

	size = buff->size;

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for getting size.");

	return size;
}

/* SEGMENT BUFFER ELEMENT */

int getSgmBuffElemStatus(SgmBuffElem *elem) {
	int status;

	if (pthread_rwlock_rdlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for getting status.");

	status = elem->status;

	if (pthread_rwlock_unlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for getting status.");

	return status;
}

void setSgmBuffElemStatus(SgmBuffElem *elem, const int status) {
	if (pthread_rwlock_wrlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff write-lock for setting status.");

	elem->status = status;

	if (pthread_rwlock_unlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for setting status.");
}

long double getSgmBuffElemElapsed(SgmBuffElem *elem) {
	struct timespec now;
	long double elapsed;

	if (pthread_rwlock_rdlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for getting elapsed.");

	clock_gettime(CLOCK_MONOTONIC, &now);

	elapsed = getElapsed(elem->time, now);

	if (pthread_rwlock_unlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for getting elapsed.");

	return elapsed;
}

short testSgmBuffElemAttributes(SgmBuffElem *elem, const short status, const long double elapsed) {
	struct timespec now;
	short result;

	if (pthread_rwlock_rdlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for testing attributes.");

	clock_gettime(CLOCK_MONOTONIC, &now);

	result = (elem->status == status) & ((getElapsed(elem->time, now) - elem->delay) > elapsed);

	if (pthread_rwlock_unlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for testing attributes.");

	return result;
}

void updateSgmBuffElemAttributes(SgmBuffElem *elem, const long retransoffset, const long double delay) {
	if (pthread_rwlock_wrlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff write-lock for updating attributes.");

	elem->retrans += retransoffset;

	clock_gettime(CLOCK_MONOTONIC, &(elem->time));

	elem->delay = delay;

	if (pthread_rwlock_unlock(&(elem->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for updating attributes.");
}

/* SEGMENT BUFFER WAITING */

void waitSgmBuffEmptiness(SgmBuff *buff) {
	if (pthread_mutex_lock(&(buff->mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getSgmBuffSize(buff) > 0)
		if (pthread_cond_wait(&(buff->remove_cnd), &(buff->mtx)) > 0)
			ERREXIT("Cannot wait for condition variable.");

	if (pthread_mutex_unlock(&(buff->mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");
}

void waitStrategicInsertion(SgmBuff *buff) {
	if (pthread_mutex_lock(&(buff->mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getSgmBuffSize(buff) == 0)
		if (pthread_cond_wait(&(buff->insert_cnd), &(buff->mtx)) > 0)
			ERREXIT("Cannot wait for condition variable.");

	if (pthread_cond_wait(&(buff->status_cnd), &(buff->mtx)) > 0)
			ERREXIT("Cannot wait for condition variable.");

	if (pthread_mutex_unlock(&(buff->mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");
}

/* SEGMENT BUFFER SEARCH */

SgmBuffElem *findSgmBuffSeqn(SgmBuff *buff, const uint32_t seqn) {
	SgmBuffElem *curr = NULL;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for search by seqn.");

	curr = buff->head;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn)
			break;

		curr = curr->next;
	}

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for search by seqn.");

	return curr;
}

SgmBuffElem *findSgmBuffAckn(SgmBuff *buff, const uint32_t ackn) {
	SgmBuffElem *curr = NULL;

	if (pthread_rwlock_rdlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot acquire sgmbuff read-lock for search by ackn.");

	curr = buff->head;

	while (curr) {

		if (RUSP_ISACKED(curr->segment.hdr.seqn, curr->segment.hdr.plds, ackn) ||
				((curr->segment.hdr.ctrl & RUSP_FIN) && RUSP_ISACKED(curr->segment.hdr.seqn, 1, ackn)))
			break;

		curr = curr->next;
	}

	if (pthread_rwlock_unlock(&(buff->rwlock)) > 0)
		ERREXIT("Cannot release sgmbuff read-write lock for search by ackn.");

	return curr;
}
