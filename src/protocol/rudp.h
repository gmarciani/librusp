#ifndef RUDP_H_
#define RUDP_H_

#include "_rudp.h"

int rudpListen(const int lport);

rudpconn_t rudpConnect(const char *ip, const int port);

rudpconn_t rudpAccept(const int lsock);

void rudpDisconnect(rudpconn_t *conn);

/* COMMUNICATIONS */

void rudpSend(rudpconn_t *conn, const char *msg);

char *rudpReceive(rudpconn_t *conn);

#endif /* RUDP_H_ */
