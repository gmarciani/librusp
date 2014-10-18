#ifndef SGMBUFF_H_
#define SGMBUFF_H_

#include "../segment/sgm.h"
#include "../../util/threadutil.h"
#include "../../util/macroutil.h"

/* SEGMENT BUFFER STRUCTURES */

typedef struct SgmBuffElem {
	short status;
	long retrans;
	struct timespec time;
	long double delay;

	Segment segment;

	struct SgmBuffElem *next;
	struct SgmBuffElem *prev;
} SgmBuffElem;

typedef struct SgmBuff {
	long size;	

	SgmBuffElem *head;
	SgmBuffElem *tail;

	pthread_mutex_t *mtx;
	pthread_cond_t *insert_cnd;
	pthread_cond_t *remove_cnd;
	pthread_cond_t *status_cnd;
} SgmBuff;

/* SEGMENT BUFFER CREATION/DISTRUCTION */

SgmBuff *createSgmBuff(void);

void freeSgmBuff(SgmBuff *buff);

/* SEGMENT BUFFER INSERTION/REMOVAL */

SgmBuffElem *addSgmBuff(SgmBuff *buff, const Segment sgm);

void removeSgmBuff(SgmBuff *buff, SgmBuffElem *elem);

/* SEGMENT BUFFER SEARCH */

SgmBuffElem *findSgmBuffSeqn(SgmBuff *buff, const uint32_t seqn);

SgmBuffElem *findSgmBuffAckn(SgmBuff *buff, const uint32_t ackn);

/* SEGMENT BUFFER REPRESENTATION */

char *sgmBuffToString(SgmBuff *buff);

#endif /* SGMBUFF_H_ */
