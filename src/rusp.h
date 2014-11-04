#ifndef RUSP_H_
#define RUSP_H_

#include "core/connection/conn.h"
#include "util/addrutil.h"
#include "util/sockutil.h"
#include "util/macroutil.h"

/* CONNECTION */

ConnectionId ruspListen(const int lport);

ConnectionId ruspAccept(const ConnectionId lconnid);

ConnectionId ruspConnect(const char *ip, const int port);

void ruspClose(const ConnectionId connid);

/* COMMUNICATIONS */

ssize_t ruspSend(const ConnectionId connid, const char *msg, const size_t msgs);

ssize_t ruspReceive(const ConnectionId connid, char *msg, const size_t msgs);

/* ADDRESS UTILITY */

int ruspLocal(const ConnectionId connid, struct sockaddr_in *addr);

int ruspPeer(const ConnectionId connid, struct sockaddr_in *addr);

/* DEV UTILITY */

double rudpGetDrop(void);

void rudpSetDrop(const double drop);

int rudpGetDebug(void);

void rudpSetDebug(const int dbg);

#endif /* RUSP_H_ */