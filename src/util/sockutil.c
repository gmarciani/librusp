#include "sockutil.h"

static double SOCK_DROP = 0.3;

/* SOCKET CREATION */

int openSocket() {
	int sock;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		ERREXIT("Cannot open socket.");

	return sock;
}

void closeSocket(const int sock) {

	if (close(sock) == -1)
		ERREXIT("Cannot close socket.");
}

void bindSocket(const int sock, const struct sockaddr_in *addr) {

	if (bind(sock, (const struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) 
		ERREXIT("Cannot bind socket.");
}

/* SOCKET I/O */

void writeUnconnectedSocket(const int sock, const struct sockaddr_in rcvaddr, const char *buff) {
	socklen_t socksize = sizeof(struct sockaddr_in);

	if (sendto(sock, buff, strlen(buff), 0, (struct sockaddr *)&rcvaddr, socksize) == -1)
		ERREXIT("Cannot write to unconnected socket.");
}

char *readUnconnectedSocket(const int sock, struct sockaddr_in *sndaddr, const size_t rcvsize) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	char *buff = NULL;
	ssize_t rcvd = 0;

	if (!(buff = malloc(sizeof(char) * (rcvsize + 1))))
		ERREXIT("Cannot allocate memory for reading unconnected socket.");

	if ((rcvd = recvfrom(sock, buff, rcvsize, 0, (struct sockaddr *)sndaddr, &socksize)) == -1) {

		if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
			free(buff);
			return NULL;
		}

		ERREXIT("Cannot read unconnected socket.");
	}

	buff[rcvd] = '\0';

	return buff;
}

void writeConnectedSocket(const int sock, const char *buff) {

	errno = 0;
	if (write(sock, buff, strlen(buff)) == -1) {

		if ((errno == EPIPE) | (errno == EINVAL))
			return;

		fprintf(stderr, "Cannot write connected socket: %s\n", strerror(errno));

		exit(EXIT_FAILURE);		
	}
}

char *readConnectedSocket(const int sock, const size_t rcvsize) {
	char *buff = NULL;
	ssize_t rcvd = 0;

	if (!(buff = malloc(sizeof(char) * (rcvsize + 1))))
		ERREXIT("Cannot allocate memory for reading connected socket.");

	errno = 0;
	if ((rcvd = read(sock, buff, rcvsize)) == -1) {

		if ((errno == EWOULDBLOCK) | (errno == EAGAIN) | (errno = EPIPE) | (errno == EINVAL)) {

			free(buff);

			return NULL;
		}

		fprintf(stderr, "Cannot read connected socket: %s\n", strerror(errno));

		exit(EXIT_FAILURE);
	}

	buff[rcvd] = '\0';

	if (getRandomBit(SOCK_DROP)) {
		free(buff);
		printf("Packet Dropping\n");
		return NULL;
	}

	return buff;
}

/* SOCKET PROPERTIES */

void setSocketConnected(const int sock, const struct sockaddr_in addr) {

	if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		ERREXIT("Cannot set socket connected.");
}

void setSocketReusable(const int sock) {
	int optval = 1;

  	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1)
		ERREXIT("Cannot set socket reusable.");
}

void setSocketTimeout(const int sock, const uint8_t mode, const uint64_t nanos) {
	struct timeval timer;

	timer.tv_sec = (time_t) ceil(nanos / 1000000000);

  	timer.tv_usec = (suseconds_t) ceil((nanos % 1000000000) % 1000);

	if (mode == (ON_READ | ON_WRITE)) {

  		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, (const void *)&timer, sizeof(timer)) < 0)
			ERREXIT("Cannot set socket read timeout.");

	} else if (mode == ON_READ) {

  		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timer, sizeof(timer)) < 0)
			ERREXIT("Cannot set socket read timeout.");

	} else if (mode == ON_WRITE) {

  		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const void *)&timer, sizeof(timer)) < 0)
      		ERREXIT("Cannot set socket write timeout.");

	} else {
		
		ERREXIT("Cannot set socket timeout: unrecognized mode.");
	}
}

/* SOCKET END-POINTS */

struct sockaddr_in getSocketLocal(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	if (getsockname(sock, (struct sockaddr *)&addr, &socksize) == -1)
		ERREXIT("Cannot get socket local address.");

	return addr;
}

struct sockaddr_in getSocketPeer(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	if (getpeername(sock, (struct sockaddr *)&addr, &socksize) == -1)
		ERREXIT("Cannot get socket peer address.");

	return addr;
}


/* UTILITY */

void setSocketDrop(const double drop) {
	SOCK_DROP = drop;
}
