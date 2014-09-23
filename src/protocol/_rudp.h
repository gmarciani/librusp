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

#include "../common/util.h"


/* SETTINGS */

#define PACKET_LOSS 3


/* PROTOCOL */

#define VERSION 1


/* CONNECTION */

#define DCONN_HARD 0
#define DCONN_SOFT 1


typedef struct rudpConnection_t {
	int sockfd;
	struct sockaddr_in local;
	struct sockaddr_in peer;
} rudpConnection_t;


/* PACKET */

#define PACKET_SERIALIZATION_SIZE 130
#define PACKET_FIELDS_DELIMITER "||"
#define PACKET_PAYLOAD_SIZE 100

#define SYN 0
#define	FIN 1
#define ACK 2
#define DAT 3

typedef struct pktheader_t {
	int control;
	int sequenceNumber;
	int streamEnd;
} pktheader_t;

typedef struct packet_t {
	pktheader_t header;
	char payload[PACKET_PAYLOAD_SIZE + 1];
} packet_t;

void _rudpPacketDeserialization(const char *serializedPacket, packet_t *packet);

char *_rudpPacketSerialization(const packet_t packet);

void _rudpParsePacket(const char *control, const char *sequenceNumber, const char *streamEnd, const char *payload, packet_t *packet);

void _rudpCreatePacket(const int control, const int sequenceNumber, const int streamEnd, const char *payload, packet_t *packet);

packet_t *_rudpPacketStream(const char *message, int *numPackets);


/* SOCKET */

int _rudpOpenSocket();

void _rudpCloseSocket(const int sockfd);

void _rudpBindSocket(const int sockfd, struct sockaddr_in *addr);

void _rudpConnectSocket(const int sockfd, const struct sockaddr_in addr);

void _rudpReusableSocket(const int sockfd);


/* COMMUNICATION */

void _rudpSendPacket(const rudpConnection_t conn, const packet_t packet);

void _rudpReceivePacket(const rudpConnection_t conn, packet_t *packet);

void __rudpSendPacket(const int sockfd, const struct sockaddr_in dest, const packet_t packet);

void __rudpReceivePacket(const int sockfd, struct sockaddr_in *source, packet_t *packet);


/* ADDRESS */

struct sockaddr_in _rudpAddress(const char *ip, const int port);

int _rudpIsEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo);

struct sockaddr_in _rudpSocketLocal(const int sockfd);

struct sockaddr_in _rudpSocketPeer(const int sockfd);

char *_rudpGetAddress(const struct sockaddr_in address);

int _rudpGetPort(const struct sockaddr_in address);


/* OUTPUT */

void _rudpPrintInPacket(const struct sockaddr_in sender, const packet_t packet);

void _rudpPrintOutPacket(const struct sockaddr_in receiver, const packet_t packet);

void _rudpPrintInDatagram(const struct sockaddr_in sender, const char *datagram);

void _rudpPrintOutDatagram(const struct sockaddr_in receiver, const char *datagram);


/* SETTING */

void _rudpDatagramResolution(const int resolution);

#endif /* _RUDP_H_ */
