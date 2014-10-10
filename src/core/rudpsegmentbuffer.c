#include "rudpsegmentbuffer.h"

/* NORMAL SEGMENT BUFFER */

SegmentBuffer *createSegmentBuffer(void) {
	SegmentBuffer *buff = NULL;

	if (!(buff = malloc(sizeof(SegmentBuffer))))
		ERREXIT("Cannot allocate memory for segment buffer.");

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeSegmentBuffer(SegmentBuffer *buff) {

	while (buff->head)
		removeSegmentBuffer(buff, buff->head);

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

	sprintf(strbuff, "size:%u content:%s", buff->size, (buff->size == 0) ? "" : "\n");

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

	if (!(buff = malloc(sizeof(TSegmentBuffer))))
		ERREXIT("Cannot allocate memory for timeout segment buffer.");

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeTSegmentBuffer(TSegmentBuffer *buff) {

	while (buff->head)
		removeTSegmentBuffer(buff, buff->head);

	free(buff);
}

TSegmentBufferElement *addTSegmentBuffer(TSegmentBuffer *buff, const Segment sgm, const uint8_t status, const uint64_t nanos, void (*handler) (union sigval), void *arg, size_t argsize) {

	TSegmentBufferElement *new = NULL;

	if (!(new = malloc(sizeof(TSegmentBufferElement))))
		ERREXIT("Cannot allocate memory for new timeout segment buffer element.");

	if (!(new->timerarg = malloc(argsize)))
		ERREXIT("Cannot allocate memory for new timeout segment buffer element resources.");

	new->segment = sgm;

	new->status = status;

	memcpy(new->timerarg, arg, argsize);

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

	new->timer = createTimer(handler, new->timerarg);

	setTimer(new->timer, nanos, nanos);

	return new;
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

void removeTSegmentBuffer(TSegmentBuffer *buff, TSegmentBufferElement *elem) {

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

	setTimer(elem->timer, 0, 0);

	freeTimer(elem->timer);

	free(elem->timerarg);

	free(elem);

	buff->size--;
}

char *tSegmentBufferToString(TSegmentBuffer *buff) {
	TSegmentBufferElement *curr = NULL;
	char *strbuff, *strbuffelem, *strsgm = NULL;
	uint64_t i;

	if (!(strbuff = malloc(sizeof(char) * (25 + buff->size * (20 + RUDP_SGMSO + 1) + 1))))
		ERREXIT("Cannot allocate memory for string representation of segment buffer.");

	sprintf(strbuff, "size:%u content:%s", buff->size, (buff->size == 0) ? "" : "\n");

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
