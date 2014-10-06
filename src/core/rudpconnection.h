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

#include "rudpinbox.h"
#include "rudpoutbox.h"
#include "rudpsegment.h"

#include "../util/stringutil.h"
#include "../util/mathutil.h"
#include "../util/listutil.h"
#include "../util/threadutil.h"
#include "../util/timerutil.h"
#include "../util/addrutil.h"
#include "../util/sockutil.h"

#define RUDP_CONN_CLOS (uint8_t) 0
#define RUDP_CONN_LIST (uint8_t) 1
#define RUDP_CONN_SYNS (uint8_t) 2
#define RUDP_CONN_SYNR (uint8_t) 3
#define RUDP_CONN_ESTA (uint8_t) 4
#define RUDP_CONN_CLSW (uint8_t) 5
#define RUDP_CONN_FINS (uint8_t) 6	
#define RUDP_CONN_FINR (uint8_t) 7

#define RUDP_TIMEO_ACK (uint64_t) 2000000000

#define RUDP_CONN_ATT (uint8_t) 3

#define RUDP_RETRANS (uint8_t)	3
#define RUDP_WNDSIZE (uint8_t)	10

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

/* CONNECTION */

typedef int ConnectionId;

typedef struct ConnectionRecord {
	uint8_t state;
	Outbox *outbox;
	Inbox *inbox;
	int sock;
	timer_t	timer;
	pthread_mutex_t *conn_mtx;
	pthread_cond_t  *conn_cnd;
} ConnectionRecord;

typedef struct Connection {
	uint8_t version;
	ConnectionId connid;
	ConnectionRecord *record;
	pthread_t manager;
} Connection;

Connection *createConnection(void);

Connection *getConnectionById(const ConnectionId connid);

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr);

ConnectionId acceptSynchonization(Connection *lconn);

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr);

void desynchronizeConnection(Connection *conn);

void destroyConnection(Connection *conn);

uint8_t getConnectionState(Connection *conn);

void setConnectionState(Connection *conn, const uint8_t state);

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg, const size_t size);

char *readInboxMessage(Connection *conn, const size_t size);

#endif /* _RUDPCONNECTION_H_ */
