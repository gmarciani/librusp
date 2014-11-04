#ifndef CONN_H_
#define CONN_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "../segment/sgm.h"
#include "../buffer/sgmbuff.h"
#include "../buffer/strbuff.h"
#include "timeo.h"
#include "wnd.h"
#include "../../util/stringutil.h"
#include "../../util/mathutil.h"
#include "../../util/listutil.h"
#include "../../util/threadutil.h"
#include "../../util/timeutil.h"
#include "../../util/addrutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define RUDP_CON_CLOSED 0
#define RUDP_CON_LISTEN 1
#define RUDP_CON_SYNSND 2
#define RUDP_CON_SYNRCV 3
#define RUDP_CON_ESTABL 4
#define RUDP_CON_FINWT1 5
#define RUDP_CON_FINWT2 6
#define RUDP_CON_CLOSIN 7
#define RUDP_CON_CLOSWT 8
#define RUDP_CON_TIMEWT 9
#define RUDP_CON_LSTACK 10

#define RUDP_SYN_RETR 5

#define RUDP_CON_RETR 3

#define RUDP_CON_WNDS 5//((RUDP_MAXSEQN / RUDP_PLDS) / 3)

#define RUDP_SAMPLRTT 1000
#define RUDP_MSLTIMEO 60000
#define RUDP_TIMEWTTM 2 * RUDP_MSLTIMEO

#define RUDP_SGM_NACK 0
#define RUDP_SGM_YACK 1

/* GLOBAL VARIABLES */

extern int RUDP_DEBUG;

extern double RUDP_DROP;

/* CONNECTION STRUCTURE */

typedef long long ConnectionId;

typedef struct ConnectionState {
	int value;
	pthread_rwlock_t rwlock;
} ConnectionState;

typedef struct ConnectionSocket {
	int fd;
	pthread_mutex_t mtx;
} ConnectionSocket;

typedef struct Connection {
	ConnectionId connid;
	ConnectionState state;
	ConnectionSocket sock;

	Window sndwnd;
	StrBuff sndusrbuff;
	SgmBuff sndsgmbuff;

	Window rcvwnd;
	StrBuff rcvusrbuff;
	SgmBuff rcvsgmbuff;

	Timeout timeout;

	pthread_t sender;
	pthread_t receiver;
} Connection;

/* CONNECTION */

Connection *createConnection(void);

void destroyConnection(Connection *conn);

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double sampleRTT);

int getConnectionState(Connection *conn);

void setConnectionState(Connection *conn, const int state);

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr);

/* SYNCHRONIZATION */

int activeOpen(Connection *conn, const struct sockaddr_in laddr);

ConnectionId passiveOpen(Connection *lconn);

/* DESYNCHRONIZATION */

void activeClose(Connection *conn);

void passiveClose(Connection *conn);

/* CONNECTIONS POOL */

Connection *getConnectionById(const ConnectionId connid);

#endif /* CONN_H_ */
