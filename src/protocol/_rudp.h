#ifndef _RUDP_H_
#define _RUDP_H_

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
#include "../common/util.h"

/* CONNECTION */

// status
#define _RUDP_CONN_CLOSED 		0
#define _RUDP_CONN_LISTEN 		1
#define _RUDP_CONN_SYN_SENT		2
#define _RUDP_CONN_SYN_RCVD		3
#define _RUDP_CONN_ESTABLISHED	4
#define _RUDP_CONN_CLOSE_WAIT	5	

//parameters
#define _RUDP_ACK_TIMEOUT		2000
#define _RUDP_MAX_RETRANS		5
#define _RUDP_WND				1000	

/* SEGMENT */

#define _RUDP_MAX_HDR 		(2 * 2) + (1 * 5) + (2 * 10)
#define _RUDP_MAX_PLD 		500
#define _RUDP_MAX_SGM		(2 * 2) + (1 * 5) + (2 * 10) + 500	
#define _RUDP_HDR_FIELDS	5

// version
#define _RUDP_VERSION 1

// control
#define _RUP_SYN 	0
#define _RUP_FIN	1
#define _RUP_ACK 	2
#define _RUP_DAT 	3
#define _RUP_ERR 	4

/* CONNECTION */

typedef struct rudpconn_t {
	unsigned short version;
	unsigned short status;
	int sock;
	struct sockaddr_in laddr;
	struct sockaddr_in paddr;
	unsigned long int seqno;	// fin dove ho inviato
	unsigned long int lastack;	// fin dove ho correttamente inviato
	unsigned long int ackno;	// fin dove ho ricevuto
	unsigned long int wnd;		// quanto posso ancora inviare prima di un ack
} rudpconn_t;

rudpconn_t _rudpConnInit();

rudpconn_t _rudpASYNHandshake(const struct sockaddr_in laddr);

rudpconn_t _rudpPSYNHandshake(const int lsock);

void _rudpFINHanshake(rudpconn_t *conn);

unsigned long int _rudpGetISN();

typedef struct rudphdr_t {
	unsigned short int vers; // 2 byte
	unsigned short int ctrl; // 2 byte
	unsigned short int plds; // 5 byte
	unsigned long int seqno; // 10 byte
	unsigned long int ackno; // 10 byte
	unsigned long int strno; // 10 byte
} rudphdr_t;

typedef struct rudpsgm_t {
	rudphdr_t hdr;
	char pld[_RUDP_MAX_PLD + 1];
} rudpsgm_t;

rudpsgm_t _rudpDeserializeSegment(const char *ssgm);

char *_rudpSerializeSegment(const rudpsgm_t sgm);

rudpsgm_t _rudpCreateSegment(const unsigned short int ctrl, unsigned long int seqno, unsigned long int ackno, unsigned long int wndno, const char *pld);

/* STREAM */

typedef struct rudpstream_t {
	rudpsgm_t *segments;
	int numsegments;
	int streamsize;
} rudpstream_t;

rupdstream_t _rudpSegmentStream(const char *msg);

void _rudpSendStream(const rudpstream_t stream);

rudpstream_t _rudpReceiveStream(rudpconn_t *conn);

/* COMMUNICATION */

void _rudpSendSegment(const rudpconn_t *conn, const rudpsgm_t sgm);

rudpsgm_t _rudpReceiveSegment(const rudpconn_t *conn);

/* SOCKET */

int _rudpOpenSocket();

void _rudpCloseSocket(const int sock);

void _rudpBindSocket(const int sock, struct sockaddr_in *addr);

void _rudpConnectSocket(const int sock, const struct sockaddr_in addr);

void _rudpReusableSocket(const int sock);

void _rudpTimeoutSocket(const int sock, const long int timeout);

/* ADDRESS */

struct sockaddr_in _rudpAddress(const char *ip, const int port);

int _rudpIsEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo);

struct sockaddr_in _rudpSocketLocal(const int sock);

struct sockaddr_in _rudpSocketPeer(const int sock);

char *_rudpGetAddress(const struct sockaddr_in addr);

int _rudpGetPort(const struct sockaddr_in addr);

/* OUTPUT */

void _rudpPrintInSegment(const struct sockaddr_in sndaddr, const rudpsgm_t sgm);

void _rudpPrintOutSegment(const struct sockaddr_in rcvaddr, const rudpsgm_t sgm);

void _rudpPrintInDatagram(const struct sockaddr_in sndaddr, const char *dtg);

void _rudpPrintOutDatagram(const struct sockaddr_in rcvaddr, const char *dtg);

#endif /* _RUDP_H_ */
