#include "sgmbuff.h"

/* SEGMENT BUFFER CREATION/DISTRUCTION */

SgmBuff *createSgmBuff(void) {
	SgmBuff *buff = NULL;

	if (!(buff = malloc(sizeof(SgmBuff))))
		ERREXIT("Cannot allocate memory for segment buffer.");

	buff->rwlock = createRWLock();

	buff->mtx = createMutex();

	buff->insert_cnd = createConditionVariable();

	buff->remove_cnd = createConditionVariable();

	buff->status_cnd = createConditionVariable();

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeSgmBuff(SgmBuff *buff) {

	while (buff->head)
		removeSgmBuff(buff, buff->head);

	freeRWLock(buff->rwlock);

	freeMutex(buff->mtx);

	freeConditionVariable(buff->insert_cnd);

	freeConditionVariable(buff->remove_cnd);

	freeConditionVariable(buff->status_cnd);

	free(buff);
}

/* SEGMENT BUFFER INSERTION/REMOVAL */

SgmBuffElem *addSgmBuff(SgmBuff *buff, const Segment sgm, const short status) {
	SgmBuffElem *new = NULL;

	if (!(new = malloc(sizeof(SgmBuffElem))))
		ERREXIT("Cannot allocate memory for new segment buffer element.");

	new->status = status;

	new->retrans = 0;

	clock_gettime(CLOCK_MONOTONIC, &(new->time));

	new->delay = 0.0;

	new->segment = sgm;

	new->rwlock = createRWLock();

	lockWrite(buff->rwlock);

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

	unlockRWLock(buff->rwlock);

	broadcastConditionVariable(buff->insert_cnd);

	return new;
}

void removeSgmBuff(SgmBuff *buff, SgmBuffElem *elem) {

	if (!elem)
		return;

	lockWrite(buff->rwlock);

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

	unlockRWLock(buff->rwlock);

	broadcastConditionVariable(buff->remove_cnd);

	freeRWLock(elem->rwlock);

	free(elem);
}

long getSgmBuffSize(SgmBuff *buff) {
	long size;

	lockRead(buff->rwlock);

	size = buff->size;

	unlockRWLock(buff->rwlock);

	return size;
}

/* SEGMENT BUFFER ELEMENT */

short getSgmBuffElemStatus(SgmBuffElem *elem) {
	short status;

	lockRead(elem->rwlock);

	status = elem->status;

	unlockRWLock(elem->rwlock);

	return status;
}

void setSgmBuffElemStatus(SgmBuffElem *elem, const short status) {
	lockWrite(elem->rwlock);

	elem->status = status;

	unlockRWLock(elem->rwlock);
}

long double getSgmBuffElemElapsed(SgmBuffElem *elem) {
	struct timespec now;
	long double elapsed;

	lockRead(elem->rwlock);

	clock_gettime(CLOCK_MONOTONIC, &now);

	elapsed = getElapsed(elem->time, now) - elem->delay;

	unlockRWLock(elem->rwlock);

	return elapsed;
}

short testSgmBuffElemAttributes(SgmBuffElem *elem, const short status, const long double elapsed) {
	struct timespec now;
	short result;

	lockRead(elem->rwlock);

	clock_gettime(CLOCK_MONOTONIC, &now);

	result = (elem->status == status) & ((getElapsed(elem->time, now) - elem->delay) < elapsed);

	unlockRWLock(elem->rwlock);

	return result;
}

void updateSgmBuffElemAttributes(SgmBuffElem *elem, const long retransoffset, const long double delay) {
	lockWrite(elem->rwlock);

	elem->retrans += retransoffset;

	clock_gettime(CLOCK_MONOTONIC, &(elem->time));

	elem->delay = delay;

	unlockRWLock(elem->rwlock);
}

/* SEGMENT BUFFER WAITING */

void waitSgmBuffEmptiness(SgmBuff *buff) {
	lockMutex(buff->mtx);

	while (getSgmBuffSize(buff) > 0)
		waitConditionVariable(buff->remove_cnd, buff->mtx);

	unlockMutex(buff->mtx);
}

void waitStrategicInsertion(SgmBuff *buff) {
	lockMutex(buff->mtx);

	while (buff->size == 0)
		waitConditionVariable(buff->insert_cnd, buff->mtx);

	waitConditionVariable(buff->status_cnd, buff->mtx);

	unlockMutex(buff->mtx);
}

/* SEGMENT BUFFER SEARCH */

SgmBuffElem *findSgmBuffSeqn(SgmBuff *buff, const uint32_t seqn) {
	SgmBuffElem *curr = NULL;

	lockRead(buff->rwlock);

	curr = buff->head;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn)
			break;

		curr = curr->next;
	}

	unlockRWLock(buff->rwlock);

	return curr;
}

SgmBuffElem *findSgmBuffAckn(SgmBuff *buff, const uint32_t ackn) {
	SgmBuffElem *curr = NULL;

	lockRead(buff->rwlock);

	curr = buff->head;

	while (curr) {

		if (RUDP_ISACKED(curr->segment.hdr.seqn, curr->segment.hdr.plds, ackn))
			break;

		curr = curr->next;
	}

	unlockRWLock(buff->rwlock);

	return curr;
}

/* SEGMENT BUFFER REPRESENTATION */

char *sgmBuffToString(SgmBuff *buff) {
	SgmBuffElem *curr = NULL;
	char *strbuff, *strsgm = NULL;	
	uint32_t i;

	if (!(strbuff = malloc(sizeof(char) * (25 + buff->size * (RUDP_SGMSO + 1) + 1))))
		ERREXIT("Cannot allocate memory for string representation of segment buffer.");

	sprintf(strbuff, "size:%ld content:%s", buff->size, (buff->size == 0) ? "" : "\n");

	curr = buff->head;

	i = (uint32_t) strlen(strbuff);

	while (curr) {

		strsgm = segmentToString(curr->segment);

		memcpy(strbuff + i, strsgm, sizeof(char) * strlen(strsgm));		

		i += (uint32_t) strlen(strsgm);

		free(strsgm);

		if (curr->next) {

			strbuff[i] = '\n';

			i++;
		}		

		curr = curr->next;
	}

	strbuff[i] = '\0';
	
	return strbuff;
}
