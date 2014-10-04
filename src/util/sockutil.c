#include "sockutil.h"

/* SOCKET CREATION */

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

void bindSocket(const int sock, const struct sockaddr_in *addr) {
	errno = 0;

	if (bind(sock, (const struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in socket binding: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/* SOCKET I/O */

void writeUnconnectedSocket(const int sock, const struct sockaddr_in rcvaddr, const char *buff) {
	socklen_t socksize = sizeof(struct sockaddr_in);

	errno = 0;

	if (sendto(sock, buff, strlen(buff), 0, (struct sockaddr *)&rcvaddr, socksize) == -1) {
		fprintf(stderr, "Cannot write to unconnected socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

char *readUnconnectedSocket(const int sock, struct sockaddr_in *sndaddr, const size_t rcvsize) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	char *buff = NULL;
	ssize_t rcvd = 0;

	if (!(buff = malloc(sizeof(char) * (rcvsize + 1)))) {
		fprintf(stderr, "Cannot allocate buffer for unconnected socket reading.\n");
		exit(EXIT_FAILURE);
	}

	errno = 0;

	if ((rcvd = recvfrom(sock, buff, rcvsize, 0, (struct sockaddr *)sndaddr, &socksize)) == -1) {

		if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
			free(buff);
			return NULL;
		}

		fprintf(stderr, "Cannot read from unconnected socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	buff[rcvd] = '\0';

	return buff;
}

void writeConnectedSocket(const int sock, const char *buff) {
	errno = 0;

	if (write(sock, buff, strlen(buff)) == -1) {
		fprintf(stderr, "Cannot write to connected socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

char *readConnectedSocket(const int sock, const size_t rcvsize) {
	char *buff = NULL;
	ssize_t rcvd = 0;

	if (!(buff = malloc(sizeof(char) * (rcvsize + 1)))) {
		fprintf(stderr, "Cannot allocate buffer for unconnected socket reading.\n");
		exit(EXIT_FAILURE);
	}

	errno = 0;

	if ((rcvd = read(sock, buff, rcvsize)) == -1) {

		if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
			free(buff);
			return NULL;
		}

		fprintf(stderr, "Cannot read from connected socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	buff[rcvd] = '\0';

	return buff;
}

/* SOCKET PROPERTIES */

void setSocketConnected(const int sock, const struct sockaddr_in addr) {
	errno = 0;

	if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Cannot set socket connected: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void setSocketReusable(const int sock) {
	int optval = 1;

	errno = 0;

  	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1) {
		fprintf(stderr, "Cannot set socket reusable: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void setSocketTimeout(const int sock, const uint8_t mode, const uint64_t nanos) {
	struct timeval timer;

	timer.tv_sec = (time_t) ceil(nanos / 1000000000);

  	timer.tv_usec = (suseconds_t) ceil((nanos % 1000000000) % 1000);

	if (mode & ON_READ) {
		errno = 0;  

  		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timer, sizeof(timer)) < 0) {
      		fprintf(stderr, "Cannot set read timeout in socket: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
  		}
	}

	if (mode & ON_WRITE) {
		errno = 0;  

  		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const void *)&timer, sizeof(timer)) < 0) {
      		fprintf(stderr, "Cannot set write timeout in socket: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
  		}
	}	
}

/* SOCKET END-POINTS */

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