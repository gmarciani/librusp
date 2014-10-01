#ifndef SOCKMNG_H_
#define SOCKMNG_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

#define ON_READ		0b01
#define ON_WRITE	0b10

/* SOCKET CREATION */

int openSocket();

void closeSocket(const int sock);

void bindSocket(const int sock, const struct sockaddr_in *addr);

/* SOCKET I/O */

void writeUnconnectedSocket(const int sock, const struct sockaddr_in rcvaddr, const char *buff);

char *readUnconnectedSocket(const int sock, struct sockaddr_in *sndaddr, const size_t rcvsize);

void writeConnectedSocket(const int sock, const char *buff);

char *readConnectedSocket(const int sock, const size_t rcvsize);

/* SOCKET PROPERTIES */

void setSocketConnected(const int sock, const struct sockaddr_in addr);

void setSocketReusable(const int sock);

void setSocketTimeout(const int sock, const int mode, const unsigned long millis);

/* SOCKET END-POINTS */

struct sockaddr_in getSocketLocal(const int sock);

struct sockaddr_in getSocketPeer(const int sock);

#endif /* SOCKMNG_H_ */
