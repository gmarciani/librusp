#ifndef _RUDPCONNECTION_H_
#define _RUDPCONNECTION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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
#include "../util/addrutil.h"
#include "../util/sockmng.h"

#define RUDP_CONN_CLOSED 		0
#define RUDP_CONN_LISTEN		1
#define RUDP_CONN_SYN_SENT		2
#define RUDP_CONN_SYN_RCVD		3
#define RUDP_CONN_ESTABLISHED	4
#define RUDP_CONN_WAITCLOSE		5
#define RUDP_CONN_FIN_SENT		6	
#define RUDP_CONN_FIN_RCVD		7

#define RUDP_SYN_TIMEOUT		6000 //millis
#define RUDP_SYNACK_TIMEOUT		2000 //milis

#define RUDP_ACK_TIMEOUT		3000 //millis
#define RUDP_MAX_RETRANS		3
#define RUDP_MAX_WNDS			10

/* CONNECTION */

typedef int ConnectionId;

typedef struct ConnectionRecord {
	unsigned short state;
	SegmentOutbox *outbox;
	SegmentInbox *inbox;
	int sock;
	pthread_mutex_t *recordmtx;
} ConnectionRecord;

typedef struct Connection {
	unsigned short version;
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

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg);

char *readInboxMessage(Connection *conn, const size_t size);

#endif /* _RUDPCONNECTION_H_ */
