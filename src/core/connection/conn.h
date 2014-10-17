#ifndef CONN_H_
#define CONN_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include "../segment/sgm.h"
#include "../buffer/sgmbuff.h"
#include "../buffer/strbuff.h"
#include "timeo.h"
#include "../../util/stringutil.h"
#include "../../util/mathutil.h"
#include "../../util/listutil.h"
#include "../../util/threadutil.h"
#include "../../util/timerutil.h"
#include "../../util/addrutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define RUDP_CON_CLOS 0
#define RUDP_CON_LIST 1
#define RUDP_CON_SYNS 2
#define RUDP_CON_SYNR 3
#define RUDP_CON_ESTA 4
#define RUDP_CON_CLSW 5
#define RUDP_CON_FINS 6	
#define RUDP_CON_FINR 7

#define RUDP_CON_ATTS 3

#define RUDP_CON_RETR 3

#define RUDP_CON_WNDS 5//((RUDP_MAXSEQN / RUDP_PLDS) / 3)

#define RUDP_SAMPLRTT 1

#define RUDP_SGM_NACK 0
#define RUDP_SGM_YACK 1

/* CONNECTION STRUCTURE */

typedef long ConnectionId;

typedef struct Connection {
	ConnectionId connid;

	short state;
	pthread_mutex_t *state_mtx;
	pthread_cond_t  *state_cnd;	

	pthread_mutex_t *sock_mtx;
	int sock;

	uint32_t sndwndb;
	uint32_t sndwnde;
	uint32_t sndnext;
	pthread_mutex_t *sndwnd_mtx;
	StrBuff *sndbuff;
	SgmBuff *sndsgmbuff;
	pthread_t sndbufferizer;
	pthread_t sndslider;

	uint32_t rcvwndb;
	uint32_t rcvwnde;	
	pthread_mutex_t *rcvwnd_mtx;
	StrBuff *rcvbuff;
	SgmBuff *rcvsgmbuff;
	pthread_t rcvbufferizer;
	pthread_t rcvslider;		

	long double extRTT;
	long double devRTT;
	long double timeo;
	timer_t timer;
	pthread_mutex_t *timeo_mtx;
} Connection;

/* CONNECTION */

Connection *createConnection(void);

void destroyConnection(Connection *conn);

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double extRTT);

short getConnectionState(Connection *conn);

void setConnectionState(Connection *conn, const short state);

long double getTimeout(Connection *conn);

void setTimeout(Connection *conn, const long double sampleRTT);

/* SEGMENT I/O */

void sendSegment(Connection *conn, Segment sgm);

int receiveSegment(Connection *conn, Segment *sgm);

/* CONNECTIONS POOL */

Connection *allocateConnectionInPool(Connection *conn);

void deallocateConnectionInPool(Connection *conn);

Connection *getConnectionById(const ConnectionId connid);

#endif /* CONN_H_ */
