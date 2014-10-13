#ifndef _RUDP_H_
#define _RUDP_H_

#include "core/rudpconn.h"
#include "util/sockutil.h"

#ifndef ERREXIT
#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)
#endif

/* CONNECTION */

ConnectionId rudpListen(const int lport);

ConnectionId rudpAccept(const ConnectionId lconnid);

ConnectionId rudpConnect(const char *ip, const int port);

void rudpDisconnect(const ConnectionId connid);

void rudpClose(const ConnectionId connid);

/* COMMUNICATIONS */

void rudpSend(const ConnectionId connid, const char *msg, const size_t size);

char *rudpReceive(const ConnectionId connid, const size_t size);

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid);

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid);

#endif /* _RUDP_H_ */
