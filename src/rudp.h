#ifndef _RUDP_H_
#define _RUDP_H_

#include "core/rudpconnection.h"

/* CONNECTION */

ConnectionId rudpListen(const int lport);

ConnectionId rudpAccept(const ConnectionId lconnid);

ConnectionId rudpConnect(const char *ip, const int port);

void rudpDisconnect(const ConnectionId connid);

void rudpClose(const ConnectionId connid);

/* COMMUNICATIONS */

void rudpSend(const ConnectionId connid, const char *msg);

char *rudpReceive(const ConnectionId connid, const size_t size);

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid);

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid);

#endif /* _RUDP_H_ */
