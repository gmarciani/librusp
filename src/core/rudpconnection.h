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
#include "../util/listutil.h"
#include "../util/threadutil.h"
#include "../util/timerutil.h"
#include "../util/addrutil.h"
#include "../util/sockutil.h"

#define RUDP_CONN_CLOSED 		(uint8_t)	0
#define RUDP_CONN_LISTEN		(uint8_t)	1
#define RUDP_CONN_SYN_SENT		(uint8_t)	2
#define RUDP_CONN_SYN_RCVD		(uint8_t)	3
#define RUDP_CONN_ESTABLISHED	(uint8_t)	4
#define RUDP_CONN_WAITCLOSE		(uint8_t)	5
#define RUDP_CONN_FIN_SENT		(uint8_t)	6	
#define RUDP_CONN_FIN_RCVD		(uint8_t)	7

// RTT = 2000000000 nanos
#define RUDP_TIMEOUT_ACK		(uint64_t) 	2000000000 // nanos (1 * RTT)

#define RUDP_MAX_CONN_ATTEMPTS	(int)		3

#define RUDP_MAX_RETRANS		(int)		3
#define RUDP_MAX_WNDS			(int)		10

/* CONNECTION */

typedef int ConnectionId;

typedef struct ConnectionRecord {
	uint8_t 			state;
	SegmentOutbox 		*outbox;
	SegmentInbox  		*inbox;
	int 				sock;
	timer_t				timer;
	pthread_mutex_t 	*conn_mtx;
	pthread_cond_t  	*conn_cnd;
} ConnectionRecord;

typedef struct Connection {
	uint8_t 			version;
	ConnectionId 		connid;
	ConnectionRecord 	*record;
	pthread_t 			manager;
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
