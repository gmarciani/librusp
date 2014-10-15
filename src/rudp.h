#ifndef RUDP_H_
#define RUDP_H_

#include "core/connection/conn.h"
#include "core/connection/connmng.h"
#include "util/addrutil.h"
#include "util/sockutil.h"
#include "util/macroutil.h"

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

#endif /* RUDP_H_ */
