#include "rudptsgmbuffer.h"

TSegmentBuffer *createTSegmentBuffer(void) {
	TSegmentBuffer *buff = NULL;

	if (!(buff = malloc(sizeof(TSegmentBuffer))))
		ERREXIT("Cannot allocate memory for timeout segment buffer.");

	buff->mtx = createMutex();

	buff->insert_cnd = createConditionVariable();

	buff->remove_cnd = createConditionVariable();

	buff->status_cnd = createConditionVariable();

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

TSegmentBufferElement *addTSegmentBuffer(TSegmentBuffer *buff, const Segment sgm, const int status, void (*handler) (union sigval), void *arg, size_t argsize) {
	TSegmentBufferElement *new = NULL;

	if (!(new = malloc(sizeof(TSegmentBufferElement))) ||
		!(elem->timerarg = malloc(argsize)))
		ERREXIT("Cannot allocate memory for new timeout segment buffer element.");	

	new->segment = sgm;	

	new->status = status;

	memcpy(elem->timerarg, arg, argsize);

	elem->timer = createTimer(handler, elem->timerarg);

	clock_gettime(CLOCK_MONOTONIC, &(timeoutsgm->time));	

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

TSegmentBufferElement *findTSegmentBufferBySequence(TSegmentBuffer *buff, const uint32_t seqn) {
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

		if (RUDP_ISACKED(curr->segment.hdr.seqn, curr->segment.hdr.plds, ackn)) {
			
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

	freeTimer(elem->timer);

	free(elem->timerarg);

	elem->timerarg = NULL;

	buff->size--;

	clock_gettime(CLOCK_MONOTONIC, &removetime);

	elapsed = getElapsed(elem->time, removetime);

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
