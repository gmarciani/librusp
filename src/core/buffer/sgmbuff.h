#ifndef SGMBUFF_H_
#define SGMBUFF_H_

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../segment/sgm.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

/* SEGMENT BUFFER STRUCTURES */

typedef struct SgmBuffElem {
	int status;
	long retrans;
	struct timespec time;
	long double delay;

	Segment segment;

	struct SgmBuffElem *next;
	struct SgmBuffElem *prev;

	pthread_rwlock_t rwlock;
} SgmBuffElem;

typedef struct SgmBuff {
	long size;	

	SgmBuffElem *head;
	SgmBuffElem *tail;

	pthread_rwlock_t rwlock;

	pthread_mutex_t mtx;
	pthread_cond_t insert_cnd;
	pthread_cond_t remove_cnd;
	pthread_cond_t status_cnd;
} SgmBuff;

/* SEGMENT BUFFER CREATION/DISTRUCTION */

void initializeSgmBuff(SgmBuff *buff);

void destroySgmBuff(SgmBuff *buff);

/* SEGMENT BUFFER INSERTION/REMOVAL */

SgmBuffElem *addSgmBuff(SgmBuff *buff, const Segment sgm, const int status);

void removeSgmBuff(SgmBuff *buff, SgmBuffElem *elem);

long getSgmBuffSize(SgmBuff *buff);

/* SEGMENT BUFFER ELEMENT */

int getSgmBuffElemStatus(SgmBuffElem *elem);

void setSgmBuffElemStatus(SgmBuffElem *elem, const int status);

long double getSgmBuffElemElapsed(SgmBuffElem *elem);

short testSgmBuffElemAttributes(SgmBuffElem *elem, const short status, const long double elapsed);

void updateSgmBuffElemAttributes(SgmBuffElem *elem, const long retransoffset, const long double delay);

/* SEGMENT BUFFER WAITING */

void waitSgmBuffEmptiness(SgmBuff *buff);

void waitStrategicInsertion(SgmBuff *buff);

/* SEGMENT BUFFER SEARCH */

SgmBuffElem *findSgmBuffSeqn(SgmBuff *buff, const uint32_t seqn);

SgmBuffElem *findSgmBuffAckn(SgmBuff *buff, const uint32_t ackn);

#endif /* SGMBUFF_H_ */
