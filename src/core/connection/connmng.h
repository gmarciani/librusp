#ifndef CONNMNG_H_
#define CONNMNG_H_

#include "conn.h"

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr);

/* SYNCHRONIZATION */

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr);

ConnectionId acceptSynchonization(Connection *lconn);

/* DESYNCHRONIZATION */

void desynchronizeConnection(Connection *conn);

/* MESSAGE I/O */

void writeMessage(Connection *conn, const char *msg, const size_t size);

char *readMessage(Connection *conn, const size_t size);

#endif /* CONNMNG_H_ */
