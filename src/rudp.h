#ifndef _RUDP_H_
#define _RUDP_H_

#include "core/rudpcore.h"

/* CONNECTION */

int rudpListen(const int lport);

Connection rudpConnect(const char *ip, const int port);

Connection rudpAccept(const int lsock);

void rudpDisconnect(Connection *conn);

/* COMMUNICATIONS */

void rudpSend(Connection *conn, const char *msg);

char *rudpReceive(Connection *conn, const size_t size);

#endif /* _RUDP_H_ */
