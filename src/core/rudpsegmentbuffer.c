#include "rudpsegmentbuffer.h"

/* NORMAL SEGMENT BUFFER */

SegmentBuffer *createSegmentBuffer(void) {
	SegmentBuffer *buff = NULL;

	if (!(buff = malloc(sizeof(SegmentBuffer))) ||
		!(buff->mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(buff->insert_cnd = malloc(sizeof(pthread_cond_t))) ||
		!(buff->remove_cnd = malloc(sizeof(pthread_cond_t))) ||
		!(buff->status_cnd = malloc(sizeof(pthread_cond_t))))
		ERREXIT("Cannot allocate memory for timeout segment buffer resources.");

	initializeMutex(buff->mtx);

	initializeConditionVariable(buff->insert_cnd);

	initializeConditionVariable(buff->remove_cnd);

	initializeConditionVariable(buff->status_cnd);

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeSegmentBuffer(SegmentBuffer *buff) {

	while (buff->head)
		removeSegmentBuffer(buff, buff->head);

	destroyMutex(buff->mtx);

	destroyConditionVariable(buff->insert_cnd);

	destroyConditionVariable(buff->remove_cnd);

	destroyConditionVariable(buff->status_cnd);

	free(buff);
}

SegmentBufferElement *addSegmentBuffer(SegmentBuffer *buff, const Segment sgm) {
	SegmentBufferElement *new = NULL;

	if (!(new = malloc(sizeof(SegmentBufferElement))))
		ERREXIT("Cannot allocate memory for new segment buffer element.");

	new->segment = sgm;

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

	return new;
}

SegmentBufferElement *findSegmentBuffer(SegmentBuffer *buff, const uint32_t seqn) {
	SegmentBufferElement *curr = buff->head;
	SegmentBufferElement *trgt = NULL;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn) {
			
			trgt = curr;

			break;
		}

		curr = curr->next;
	}

	return trgt;
}

void removeSegmentBuffer(SegmentBuffer *buff, SegmentBufferElement *elem) {

	if (!elem)
		return;

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

	free(elem);

	buff->size--;
}

char *segmentBufferToString(SegmentBuffer *buff) {
	SegmentBufferElement *curr = NULL;
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

/* TIMEOUT SEGMENT BUFFER */

TSegmentBuffer *createTSegmentBuffer(void) {
	TSegmentBuffer *buff = NULL;

	if (!(buff = malloc(sizeof(TSegmentBuffer))) ||
		!(buff->mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(buff->insert_cnd = malloc(sizeof(pthread_cond_t))) ||
		!(buff->remove_cnd = malloc(sizeof(pthread_cond_t))) ||
		!(buff->status_cnd = malloc(sizeof(pthread_cond_t))))
		ERREXIT("Cannot allocate memory for timeout segment buffer resources.");

	initializeMutex(buff->mtx);

	initializeConditionVariable(buff->insert_cnd);

	initializeConditionVariable(buff->remove_cnd);

	initializeConditionVariable(buff->status_cnd);

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeTSegmentBuffer(TSegmentBuffer *buff) {

	while (buff->head)
		removeTSegmentBuffer(buff, buff->head);

	destroyMutex(buff->mtx);

	destroyConditionVariable(buff->insert_cnd);

	destroyConditionVariable(buff->remove_cnd);

	destroyConditionVariable(buff->status_cnd);

	free(buff);
}

TSegmentBufferElement *addTSegmentBuffer(TSegmentBuffer *buff, const Segment sgm, const int status) {
	TSegmentBufferElement *new = NULL;

	if (!(new = malloc(sizeof(TSegmentBufferElement))))
		ERREXIT("Cannot allocate memory for new timeout segment buffer element.");	

	new->segment = sgm;

	new->status = status;	

	clock_gettime(CLOCK_MONOTONIC, &(new->addtime));

	new->timerarg = NULL;

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

	return new;
}

void setTSegmentBufferElementTimeout(TSegmentBufferElement *elem, const long double value, const long double ivalue, void (*handler) (union sigval), void *arg, size_t argsize) {

	if (!(elem->timerarg = malloc(argsize)))
		ERREXIT("Cannot allocate memory for segment buffer element timeout resources.");

	memcpy(elem->timerarg, arg, argsize);

	elem->timer = createTimer(handler, elem->timerarg);

	setTimer(elem->timer, value, ivalue);
}

TSegmentBufferElement *findTSegmentBuffer(TSegmentBuffer *buff, const uint32_t seqn) {
	TSegmentBufferElement *curr = buff->head;
	TSegmentBufferElement *trgt = NULL;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn) {
			
			trgt = curr;

			break;
		}

		curr = curr->next;
	}

	return trgt;
}

TSegmentBufferElement *findTSegmentBufferByAck(TSegmentBuffer *buff, const uint32_t ackn) {
	TSegmentBufferElement *curr = buff->head;
	TSegmentBufferElement *trgt = NULL;

	while (curr) {

		if (ackn == RUDP_NXTSEQN(curr->segment.hdr.seqn, curr->segment.hdr.plds)) {
			
			trgt = curr;

			break;
		}

		curr = curr->next;
	}

	return trgt;
}

long double removeTSegmentBuffer(TSegmentBuffer *buff, TSegmentBufferElement *elem) {
	struct timespec removetime;
	long double elapsed;

	if (!elem)
		return -1;

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

	if (elem->timerarg != NULL) {

		freeTimer(elem->timer);

		//free(elem->timerarg);

		//elem->timerarg = NULL;
	}		

	buff->size--;

	clock_gettime(CLOCK_MONOTONIC, &removetime);

	elapsed = getElapsed(elem->addtime, removetime);

	free(elem);

	return elapsed;
}

char *tSegmentBufferToString(TSegmentBuffer *buff) {
	TSegmentBufferElement *curr = NULL;
	char *strbuff, *strbuffelem, *strsgm = NULL;
	uint64_t i;

	if (!(strbuff = malloc(sizeof(char) * (25 + buff->size * (20 + RUDP_SGMSO + 1) + 1))))
		ERREXIT("Cannot allocate memory for string representation of segment buffer.");

	sprintf(strbuff, "size:%ld content:%s", buff->size, (buff->size == 0) ? "" : "\n");

	curr = buff->head;

	i = strlen(strbuff);

	while (curr) {

		strsgm = segmentToString(curr->segment);

		if (!(strbuffelem = malloc(sizeof(char) * (20 + RUDP_SGMSO + 1))))
			ERREXIT("Cannot allocate memory for string representation of element of segment buffer.");

		sprintf(strbuffelem, "status:%u segment:%s", curr->status, strsgm);

		free(strsgm);

		memcpy(strbuff + i, strbuffelem, sizeof(char) * strlen(strbuffelem));

		i += strlen(strbuffelem);

		if (curr->next) {

			strbuff[i] = '\n';

			i++;
		}

		free(strbuffelem);

		curr = curr->next;
	}

	strbuff[i] = '\0';
	
	return strbuff;
}
