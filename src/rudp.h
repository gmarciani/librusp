#ifndef _RUDP_H_
#define _RUDP_H_

#include "core/rudpcore.h"

/* CONNECTION */

ConnectionId rudpListen(const int lport);

ConnectionId rudpConnect(const char *ip, const int port);

ConnectionId rudpAccept(const ConnectionId lconnid);

void rudpDisconnect(const ConnectionId connid);

void rudpClose(const ConnectionId connid);

/* COMMUNICATIONS */

void rudpSend(const ConnectionId connid, const char *msg);

char *rudpReceive(const ConnectionId connid, const size_t size);

/* SETTINGS */

void setRUDPDebugMode(const int mode);

#endif /* _RUDP_H_ */
