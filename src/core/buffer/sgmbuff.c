#include "sgmbuff.h"

/* SEGMENT BUFFER CREATION/DISTRUCTION */

SgmBuff *createSgmBuff(void) {
	SgmBuff *buff = NULL;

	if (!(buff = malloc(sizeof(SgmBuff))))
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

void freeSgmBuff(SgmBuff *buff) {

	while (buff->head)
		removeSgmBuff(buff, buff->head);

	destroyMutex(buff->mtx);

	destroyConditionVariable(buff->insert_cnd);

	destroyConditionVariable(buff->remove_cnd);

	destroyConditionVariable(buff->status_cnd);

	free(buff);
}

/* SEGMENT BUFFER INSERTION/REMOVAL */

SgmBuffElem *addSgmBuff(SgmBuff *buff, const Segment sgm) {
	SgmBuffElem *new = NULL;

	if (!(new = malloc(sizeof(SgmBuffElem))))
		ERREXIT("Cannot allocate memory for new segment buffer element.");

	new->segment = sgm;

	new->retrans = 0;

	new->delay = 0;

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

void removeSgmBuff(SgmBuff *buff, SgmBuffElem *elem) {

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

/* SEGMENT BUFFER SEARCH */

SgmBuffElem *findSgmBuffSeqn(SgmBuff *buff, const uint32_t seqn) {
	SgmBuffElem *curr = buff->head;

	while (curr) {

		if (curr->segment.hdr.seqn == seqn)
			break;

		curr = curr->next;
	}

	return curr		;
}

SgmBuffElem *findSgmBuffAckn(SgmBuff *buff, const uint32_t ackn) {
	SgmBuffElem *curr = buff->head;

	while (curr) {

		if (RUDP_ISACKED(curr->segment.hdr.seqn, curr->segment.hdr.plds, ackn))
			break;

		curr = curr->next;
	}

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
