#include "sortsgmbuff.h"

/* SORTED SEGMENT BUFFER CREATION/DISTRUCTION */

SSgmBuff *createSegmentBuffer(void) {
	SSgmBuff *buff = NULL;

	if (!(buff = malloc(sizeof(SSgmBuff))))
		ERREXIT("Cannot allocate memory for segment buffer.");

	buff->mtx = createMutex();

	buff->insert_cnd = createConditionVariable();

	buff->remove_cnd = createConditionVariable();

	buff->status_cnd = createConditionVariable();

	buff->size = 0;
	
	buff->head = NULL;

	buff->tail = NULL;

	return buff;
}

void freeSegmentBuffer(SSgmBuff *buff) {

	while (buff->head)
		removeSegmentBuffer(buff, buff->head);

	destroyMutex(buff->mtx);

	destroyConditionVariable(buff->insert_cnd);

	destroyConditionVariable(buff->remove_cnd);

	destroyConditionVariable(buff->status_cnd);

	free(buff);
}

/* SORTED SEGMENT BUFFER INSERTION/REMOVAL */

SSgmBuffElem *addSegmentBuffer(SSgmBuff *buff, const Segment sgm) {
	SSgmBuffElem *new, *curr= NULL;

	if (!(new = malloc(sizeof(SSgmBuffElem))))
		ERREXIT("Cannot allocate memory for new segment buffer element.");

	new->segment = sgm;

	if (buff->size == 0) {

		new->prev = NULL;

		new->next = NULL;

		buff->head = new;

		buff->tail = new;

	} else {

		if (new->segment.hdr.seqn < buff->head->segment.hdr.seqn) {

			new->next = buff->head;

			new->prev = NULL;

			buff->head->prev = new;

			buff->head = new;

		} else if (new->segment.hdr.seqn > buff->tail->segment.hdr.seqn) {

			new->next = NULL;

			new->prev = buff->tail;

			buff->tail->next = new;

			buff->tail = new;

		} else {

			curr = buff->head;

			while (curr) {

				if (new->segment.hdr.seqn < curr->segment.hdr.seqn)
					break;

				curr = curr->next;
			}

			new->next = curr;

			new->prev = curr->prev;

			curr->prev = new;
		}
	} 

	buff->size++;

	return new;
}

void removeSegmentBuffer(SSgmBuff *buff, SSgmBuffElem *elem) {

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

/* SORTED SEGMENT BUFFER SEARCH */

SSgmBuffElem *findSegmentBufferBySequence(SSgmBuff *buff, const uint32_t seqn) {
	SSgmBuffElem *curr = buff->head;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn)
			break;

		curr = curr->next;
	}

	return curr;
}

SSgmBuffElem *findSegmentBufferByAck(SSgmBuff *buff, const uint32_t ackn) {
	SSgmBuffElem *curr = buff->head;

	while (curr) {

		if (RUDP_ISACKED(curr->segment.hdr.seqn, curr->segment.hdr.plds, ackn))
			break;

		curr = curr->next;
	}

	return curr;
}

/* SORTED SEGMENT BUFFER REPRESENTATION */

char *segmentBufferToString(SSgmBuff *buff) {
	SSgmBuffElem *curr = NULL;
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
