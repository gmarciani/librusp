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

#define CLOSED 		0
#define LISTEN 		1
#define SYN-SENT	2
#define SYN-RCVD	3
#define OPEN		4
#define CLOSE-WAIT	5	

typedef struct rudpconn_t {
	struct sockaddr_in local;
	struct sockaddr_in peer;
	int sock;
	unsigned short version;
	unsigned short state;	
	unsigned long int acktimeout;
	unsigned long int conntimeout;
	unsigned long int localseqno;
	unsigned long int peerseqno;	
	rudppkt_t *uordpkt;
} rudpconn_t;

/* SEGMENT */

#define _RUDP_MAX_HDR 		(5 * 5) + (4 * 10)
#define _RUDP_MAX_PLD 		100
#define _RUDP_MAX_SGM		(5 * 5) + (4 * 10) + 100		
#define _RUDP_SGM_DEL	 	"|"

// version
#define _RUDP_VERSION 1

// control
#define SYN 0
#define	FIN 1
#define ACK 2
#define DAT 3
#define ERR 5

typedef struct rudphdr_t {
	unsigned short int vers;
	unsigned short int ctrl;
	unsigned short int plds;	
	unsigned long int sqnno;
	unsigned long int ackno;
	unsigned long int wndno;	
} rudphdr_t;

typedef struct rudpsgm_t {
	rudphdr_t hdr;
	char pld[_RUDP_MAX_PLD + 1];
} rudpsgm_t;

rudpsgm_t _rudpDeserializeSegment(const char *ssgm);

char *_rudpSerializeSegment(const rudpsgm_t sgm);

rudpsgm_t _rudpCreateSegment(const unsigned short int ctrl, unsigned long int seqno, unsigned long int ackno, unsigned long int wndno, const char *pld);

/* COMMUNICATION */

void _rudpSendSegment(const rudpconn_t conn, const rudpsgm_t sgm);

rudpsgm_t _rudpReceiveSegment(const rudpconn_t conn);

void __rudpSendSegment(const int sock, const struct sockaddr_in rcvaddr, const rudpsgm_t sgm);

rudpsgm_t __rudpReceiveSegment(const int sock, struct sockaddr_in *sndaddr);

/* SOCKET */

int _rudpOpenSocket();

void _rudpCloseSocket(const int sock);

void _rudpBindSocket(const int sock, struct sockaddr_in *addr);

void _rudpConnectSocket(const int sock, const struct sockaddr_in addr);

void _rudpReusableSocket(const int sock);

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
