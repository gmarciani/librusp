#ifndef _RUDP_H_
#define _RUDP_H_

#include "core/rudpcore.h"

/* CONNECTION */

int rudpListen(const int lport);

int rudpConnect(const char *ip, const int port);

int rudpAccept(const int lsock);

void rudpDisconnect(const int connid);

/* COMMUNICATIONS */

void rudpSend(const int connid, const char *msg);

char *rudpReceive(const int connid, const size_t size);

/* SETTINGS */

void setRUDPDebugMode(const int mode);

#endif /* _RUDP_H_ */
