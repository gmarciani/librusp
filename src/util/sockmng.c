#include "sockmng.h"

int openSocket() {
	int sock;

	errno = 0;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Error in socket creation: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return sock;
}

void closeSocket(const int sock) {
	errno = 0;
	if (close(sock) == -1) {
		fprintf(stderr, "Error in closing socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void bindSocket(const int sock, struct sockaddr_in *addr) {
	errno = 0;
	if (bind(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in socket binding: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void setSocketConnected(const int sock, const struct sockaddr_in addr) {
	errno = 0;
	if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in connecting socket: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void setSocketReusable(const int sock) {
	int optval = 1;

	errno = 0;
  	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1) {
		fprintf(stderr, "Error in setsockopt: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void setSocketTimeout(const int sock, const long int timeout) {
	struct timeval timer;

	timer.tv_sec = 0;
  	timer.tv_usec = timeout;

	errno = 0;  
  	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&timer,sizeof(timer)) < 0) {
      	fprintf(stderr, "Error in setsockopt: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
  	}
}

struct sockaddr_in getSocketLocal(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getsockname(sock, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getsockname: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}

struct sockaddr_in getSocketPeer(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getpeername(sock, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getpeername: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}
