#ifndef __RUDPCORE_H_
#define __RUDPCORE_H_

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

#include "rudpsegment.h"
#include "rudpqueue.h"
#include "../util/stringutil.h"
#include "../util/addrutil.h"
#include "../util/sockmng.h"

#define _RUDP_CONN_CLOSED 		0
#define _RUDP_CONN_LISTEN 		1
#define _RUDP_CONN_SYN_SENT		2
#define _RUDP_CONN_SYN_RCVD		3
#define _RUDP_CONN_ESTABLISHED	4
#define _RUDP_CONN_CLOSE_WAIT	5	

#define _RUDP_ACK_TIMEOUT		2000
#define _RUDP_MAX_RETRANS		5
#define _RUDP_WND				1000	

typedef struct Connection {
	unsigned short version;
	unsigned short status;
	int sock;
	struct sockaddr_in laddr;
	struct sockaddr_in paddr;
	sgmqueue_t unacksgm;
	unsigned long int seqno;	// fin dove ho inviato
	unsigned long int lastack;	// fin dove ho correttamente inviato
	unsigned long int ackno;	// fin dove ho ricevuto
	unsigned long int wnd;		// quanto posso ancora inviare prima di un ack
} Connection;

Connection createConnection();

Connection synchronizeConnection(struct sockaddr_in laddr);

Connection acceptSynchonization(const int lsock);

void desynchronizeConnection(Connection *conn);
	
void destroyConnection(Connection *conn);

unsigned long int getISN();

void sendStream(Connection *conn, const Stream stream);

char *receiveStream(Connection *conn, const size_t size);

/* COMMUNICATION */

void sendSegment(Connection *conn, Segment *sgm);

Segment receiveSegment(Connection *conn);

/* SETTING */

void setDebugMode(const int mode);

#endif /* __RUDPCORE_H_ */
