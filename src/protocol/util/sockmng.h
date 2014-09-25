#ifndef SOCKMNG_H_
#define SOCKMNG_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

int openSocket();

void closeSocket(const int sock);

void bindSocket(const int sock, struct sockaddr_in *addr);

void setSocketConnected(const int sock, const struct sockaddr_in addr);

void setSocketReusable(const int sock);

void setSocketTimeout(const int sock, const long int timeout);

struct sockaddr_in getSocketLocal(const int sock);

struct sockaddr_in getSocketPeer(const int sock);

#endif /* SOCKMNG_H_ */
