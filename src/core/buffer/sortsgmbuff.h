#ifndef SORTSGMBUFF_H_
#define SORTSGMBUFF_H_

#include "../segment/sgm.h"
#include "../../util/threadutil.h"
#include "../../util/timerutil.h"
#include "../../util/macroutil.h"

/* SORTED SEGMENT BUFFER STRUCTURES */

typedef struct SSgmBuffElem {
	Segment segment;

	struct SSgmBuffElem *next;
	struct SSgmBuffElem *prev;
} SSgmBuffElem;

typedef struct SSgmBuff {
	long size;	

	SSgmBuffElem *head;
	SSgmBuffElem *tail;

	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
	pthread_cond_t *status_cnd;
} SSgmBuff;

/* SORTED SEGMENT BUFFER CREATION/DISTRUCTION */

SSgmBuff *createSegmentBuffer(void);

void freeSegmentBuffer(SSgmBuff *buff);

/* SORTED SEGMENT BUFFER INSERTION/REMOVAL */

SSgmBuffElem *addSegmentBuffer(SSgmBuff *buff, const Segment sgm);

void removeSegmentBuffer(SSgmBuff *buff, SSgmBuffElem *elem);

/* SORTED SEGMENT BUFFER SEARCH */

SSgmBuffElem *findSegmentBufferBySequence(SSgmBuff *buff, const uint32_t seqn);

SSgmBuffElem *findSegmentBufferByAck(SSgmBuff *buff, const uint32_t ackn);

/* SORTED SEGMENT BUFFER REPRESENTATION */

char *segmentBufferToString(SSgmBuff *buff);

#endif /* SORTSGMBUFF_H_ */
