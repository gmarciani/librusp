#ifndef RUDP_H_
#define RUDP_H_

#include "_rudp.h"


int rudpListen(const int lport);

rudpConnection_t rudpConnect(const struct sockaddr_in saddr);

rudpConnection_t rudpAccept(const int lsock);

void rudpDisconnect(rudpConnection_t *conn, int mode);

void rudpCloseSocket(const int sockfd);


/* COMMUNICATIONS */

void rudpSend(const rudpConnection_t conn, const char *message);

char *rudpReceive(const rudpConnection_t conn);


/* ADDRESS */

struct sockaddr_in rudpAddress(const char *ip, const int port);

int rudpIsEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo);

struct sockaddr_in rudpSocketAddress(const int sockfd);

char *rudpGetAddress(const struct sockaddr_in address);

int rudpGetPort(const struct sockaddr_in address);


/* SETTING */

void rudpPacketResolution(const int resolution);

#endif /* RUDP_H_ */
