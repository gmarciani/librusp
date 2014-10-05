#ifndef _RUDPINBOX_H_
#define _RUDPINBOX_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "rudpsegment.h"
#include "rudpsegmentlist.h"
#include "../util/stringutil.h"
#include "../util/threadutil.h"

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

typedef struct Inbox {
	uint32_t wndb;
	uint32_t wnde;
	uint32_t wnds;
	SegmentList *sgmbuff;	
	Buffer *waitbuff;	
	Buffer *userbuff;
	pthread_mutex_t *inbox_mtx;
	pthread_cond_t *inbox_cnd;
} Inbox;

Inbox *createInbox(const uint32_t wndb, const uint32_t wnds);

void freeInbox(Inbox *inbox);

void submitSegmentToInbox(Inbox *inbox, const Segment sgm);

char *readUserBuffer(Inbox *inbox, const size_t size);

char *inboxToString(Inbox *inbox);

#endif /* _RUDPINBOX_H_ */
