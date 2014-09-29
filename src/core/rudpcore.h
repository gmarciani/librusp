#ifndef _RUDPCORE_H_
#define _RUDPCORE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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

#define RUDP_MAX_CONNECTIONS	10

#define RUDP_CONN_CLOSED 		0
#define RUDP_CONN_SYN_SENT		1
#define RUDP_CONN_SYN_RCVD		2
#define RUDP_CONN_ESTABLISHED	3
#define RUDP_CONN_WAITCLOSE		4
#define RUDP_CONN_FIN_SENT		5	
#define RUDP_CONN_FIN_RCVD		6

#define RUDP_ACK_TIMEOUT		2000
#define RUDP_MAX_RETRANS		10
#define RUDP_MAX_WNDS			10

typedef int ConnectionId;

typedef struct ConnectionRecord {
	unsigned short status;
	struct sockaddr_in laddr;
	struct sockaddr_in paddr;	
	SegmentOutbox *outbox;
	SegmentInbox *inbox;
	int sock;
} ConnectionRecord;

typedef struct Connection {
	unsigned short version;
	ConnectionId connid;
	ConnectionRecord record;
	pthread_t manager;
} Connection;

Connection *createConnection(void);

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr);

ConnectionId acceptSynchonization(Connection *lconn);

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr);

void desynchronizeConnection(Connection *conn);

void destroyConnection(Connection *conn);

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg);

char *readInboxMessage(Connection *conn, const size_t size);

/* SEGMENT COMMUNICATION */

void sendSegment(Connection *conn, Segment sgm);

Segment receiveSegment(Connection *conn);

/* UTILITY */

Connection *getConnectionById(const ConnectionId connid);

uint32_t getISN();

/* SETTING */

void setRUDPCoreDebugMode(const int mode);

#endif /* _RUDPCORE_H_ */
