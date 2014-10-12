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

#define RUDP_CON_CLOS (uint8_t) 0
#define RUDP_CON_LIST (uint8_t) 1
#define RUDP_CON_SYNS (uint8_t) 2
#define RUDP_CON_SYNR (uint8_t) 3
#define RUDP_CON_ESTA (uint8_t) 4
#define RUDP_CON_CLSW (uint8_t) 5
#define RUDP_CON_FINS (uint8_t) 6	
#define RUDP_CON_FINR (uint8_t) 7

#define RUDP_CON_ATTS (uint8_t) 3

#define RUDP_CON_RETR (uint32_t) 3
#define RUDP_CON_WNDS (uint32_t) 20

#define RUDP_TIME_ACK (uint64_t) 700000000
#define RUDP_TIME_NOW (uint64_t) 100

#define RUDP_SGM_NACK (uint8_t ) 0
#define RUDP_SGM_YACK (uint8_t ) 1

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

/* CONNECTION STRUCTURES */

typedef int ConnectionId;

typedef struct Connection {
	ConnectionId connid;

	uint8_t state;
	pthread_mutex_t *state_mtx;
	pthread_cond_t  *state_cnd;	

	pthread_mutex_t *sock_mtx;
	int sock;

	uint32_t sndwndb;
	uint32_t sndwnde;
	uint32_t sndnext;
	uint64_t sndtime;
	Buffer *sndbuff;
	TSegmentBuffer *sndsgmbuff;
	pthread_mutex_t *sndwnd_mtx;
	pthread_mutex_t *sndnext_mtx;
	pthread_t sender;

	uint32_t rcvwndb;
	uint32_t rcvwnde;	
	Buffer *rcvbuff;	
	SegmentBuffer *rcvsgmbuff;	
	pthread_mutex_t *rcvwnd_mtx;
	pthread_t receiver;
		
} Connection;

/* CONNECTION */

Connection *createConnection(void);

Connection *getConnectionById(const ConnectionId connid);

uint8_t getConnectionState(Connection *conn);

void setConnectionState(Connection *conn, const uint8_t state);

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

typedef struct SegmentObject {
	Connection *conn;
	Segment sgm;
} SegmentObject;

typedef struct AckObject {
	Connection *conn;
	uint32_t ackn;
} AckObject;

#endif /* _RUDPCONNECTION_H_ */
