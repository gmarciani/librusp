#ifndef RUDP_H_
#define RUDP_H_

#include "core/connection/conn.h"
#include "util/addrutil.h"
#include "util/sockutil.h"
#include "util/macroutil.h"

/* CONNECTION */

ConnectionId rudpListen(const int lport);

ConnectionId rudpAccept(const ConnectionId lconnid);

ConnectionId rudpConnect(const char *ip, const int port);

void rudpClose(const ConnectionId connid);

/* COMMUNICATIONS */

ssize_t rudpSend(const ConnectionId connid, const char *msg, const size_t size);

ssize_t rudpReceive(const ConnectionId connid, char *msg, const size_t size);

/* ADDRESS UTILITY */

int rudpGetLocalAddress(const ConnectionId connid, struct sockaddr_in *addr);

int rudpGetPeerAddress(const ConnectionId connid, struct sockaddr_in *addr);

/* DEV UTILITY */

double rudpGetDrop(void);

void rudpSetDrop(const double drop);

int rudpGetDebug(void);

void rudpSetDebug(const int debug);

#endif /* RUDP_H_ */
