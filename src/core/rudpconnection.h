#ifndef _RUDPCONNECTION_H_
#define _RUDPCONNECTION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libconfig.h>
#include <netdb.h>
#include <limits.h>
#include <pthread.h>

#include "rudpsegment.h"
#include "rudpsegmentbuffer.h"

#include "../util/stringutil.h"
#include "../util/mathutil.h"
#include "../util/listutil.h"
#include "../util/threadutil.h"
#include "../util/timerutil.h"
#include "../util/addrutil.h"
#include "../util/sockutil.h"

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
#define RUDP_CON_WNDS 5

#define RUDP_SAMPLRTT 1

#define RUDP_SGM_NACK 0
#define RUDP_SGM_YACK 1

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

/* CONNECTION STRUCTURES */

typedef int ConnectionId;

typedef struct Connection {
	ConnectionId connid;

	int state;
	pthread_mutex_t *state_mtx;
	pthread_cond_t  *state_cnd;	

	pthread_mutex_t *sock_mtx;
	int sock;

	long double extRTT;
	long double devRTT;
	long double timeout;
	pthread_mutex_t *timeout_mtx;

	uint32_t sndwndb;
	uint32_t sndwnde;
	uint32_t sndnext;
	Buffer *sndbuff;
	TSegmentBuffer *sndsgmbuff;
	pthread_t sndbufferizer;
	pthread_t sndslider;

	uint32_t rcvwndb;
	uint32_t rcvwnde;	
	Buffer *rcvbuff;	
	SegmentBuffer *rcvsgmbuff;	
	pthread_t rcvbufferizer;
	pthread_t rcvslider;
		
} Connection;

/* CONNECTION */

Connection *createConnection(void);

Connection *getConnectionById(const ConnectionId connid);

int getConnectionState(Connection *conn);

void setConnectionState(Connection *conn, const int state);

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr);

/* SYNCHRONIZATION */

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr);

ConnectionId acceptSynchonization(Connection *lconn);

/* DESYNCHRONIZATION */

void desynchronizeConnection(Connection *conn);

void destroyConnection(Connection *conn);

/* MESSAGE COMMUNICATION */

void writeMessage(Connection *conn, const char *msg, const size_t size);

char *readMessage(Connection *conn, const size_t size);

/* CONNECTION THREADS */

typedef struct TimeoutObject {
	Connection *conn;
	TSegmentBufferElement *elem;
} TimeoutObject;

#endif /* _RUDPCONNECTION_H_ */
