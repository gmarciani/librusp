#ifndef _RUDPCORE_H_
#define _RUDPCORE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libconfig.h>
#include <netdb.h>
#include <limits.h>

#include "rudpinbox.h"
#include "rudpoutbox.h"
#include "rudpsegment.h"
#include "../util/stringutil.h"
#include "../util/addrutil.h"
#include "../util/sockmng.h"

#define _RUDP_MAX_CONNECTIONS	10

#define _RUDP_CONN_CLOSED 		0
#define _RUDP_CONN_SYN_SENT		1
#define _RUDP_CONN_SYN_RCVD		2
#define _RUDP_CONN_ESTABLISHED	3
#define _RUDP_CONN_FIN_SENT		4	
#define _RUDP_CONN_FIN_RCVD		5

#define _RUDP_ACK_TIMEOUT		2000
#define _RUDP_MAX_RETRANS		5
#define _RUDP_WND				1000

typedef struct ConnectionRecord {
	unsigned short status;
	struct sockaddr_in laddr;
	struct sockaddr_in paddr;	
	SegmentOutbox outbox;
	SegmentInbox inbox;
	int sock;
} ConnectionRecord;

typedef struct Connection {
	int connid;
	unsigned short version;	
	ConnectionRecord record;
	pthread_t manager;
} Connection;

int synchronizeConnection(const struct sockaddr_in laddr);

int acceptSynchonization(const int lsock);

void desynchronizeConnection(const int connid);

Connection *_createConnection(void);

void _destroyConnection(Connection *conn);

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(const int connid, const char *msg);

char *readInboxMessage(const int connid, const size_t size);

/* SEGMENT COMMUNICATION */

void sendSegment(Connection *conn, Segment sgm);

void flushOutbox(Connection *conn);

Segment receiveSegment(Connection *conn);

/* UTILITY */

int getConnectionStatus(const int connid);

Connection *_getConnectionById(const int connid);

unsigned long int _getISN();

/* SETTING */

void setRUDPCoreDebugMode(const int mode);

#endif /* _RUDPCORE_H_ */
