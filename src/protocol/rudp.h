#ifndef RUDP_H_
#define RUDP_H_

#include "_rudp.h"

int rudpListen(const int lport);

rudpconn_t rudpConnect(const char *ip, const int port);

rudpconn_t rudpAccept(const int lport);

void rudpDisconnect(rudpconn_t *conn);

/* COMMUNICATIONS */

void rudpSend(rudpconn_t *conn, const char *msg);

char *rudpReceive(rudpconn_t *conn, size_t rcvsize);

#endif /* RUDP_H_ */
