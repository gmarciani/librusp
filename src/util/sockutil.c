#include "sockutil.h"

/* SOCKET CREATION */

int openSocket() {
	int sock;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		ERREXIT("Cannot open socket.");

	return sock;
}

void closeSocket(const int sock) {
	errno = 0;

	if (close(sock) == -1)
		ERREXIT("Cannot close socket: %s", strerror(errno));
}

void bindSocket(const int sock, const struct sockaddr_in *addr) {

	if (bind(sock, (const struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) 
		ERREXIT("Cannot bind socket.");
}

/* SOCKET I/O */

ssize_t writeUSocket(const int sock, const struct sockaddr_in rcvaddr, const char *buff, const size_t size) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	ssize_t wr;

	errno = 0;
	if ((wr = sendto(sock, buff, size, 0, (struct sockaddr *)&rcvaddr, socksize)) != size)
		ERREXIT("Cannot write unconnected socket: %s.", strerror(errno));

	return wr;
}

ssize_t readUSocket(const int sock, struct sockaddr_in *sndaddr, char *buff, const size_t size) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	ssize_t rd;

	errno = 0;
	if ((rd = recvfrom(sock, buff, size, 0, (struct sockaddr *)sndaddr, &socksize)) == -1)
		ERREXIT("Cannot read unconnected socket: %s.", strerror(errno));

	return rd;
}

int writeCSocket(const int sock, const char *buff, const size_t size) {
	int wr;

	errno = 0;
	if ((wr = write(sock, buff, strlen(buff))) == -1)
		if (errno != 111)
			ERREXIT("Cannot write connected socket (%d): %s.", errno, strerror(errno));

	return wr;
}

int readCSocket(const int sock, char *buff, const size_t size) {
	int rd;

	errno = 0;
	if ((rd = read(sock, buff, size)) == -1)
		if (errno != 111)
			ERREXIT("Cannot read connected socket (%d): %s\n", errno, strerror(errno));

	return rd;
}

/* SOCKET MULTIPLEXING */

int selectSocket(const int sock, long double millis) {
	fd_set readsock;
	struct timespec timer;
	int result;

	FD_ZERO(&readsock);

	FD_SET(sock, &readsock);

	timer = getTimespec(millis);

	errno = 0;

	if ((result = pselect(sock + 1, &readsock, NULL, NULL, &timer, NULL)) == -1)
		if (errno != EINTR)
			ERREXIT("Cannot select socket: %s (timeout: %LF).", strerror(errno), millis);

	return result;
}

/* SOCKET PROPERTIES */

void setSocketConnected(const int sock, const struct sockaddr_in addr) {
	errno = 0;

	if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		ERREXIT("Cannot set socket connected: %s", strerror(errno));
}

void setSocketReusable(const int sock) {
	int optval = 1;

	errno = 0;

  	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1)
		ERREXIT("Cannot set socket reusable: %s", strerror(errno));
}

void setSocketTimeout(const int sock, const uint8_t mode, const long double millis) {
	struct timeval timer;

	timer = getTimeval(millis);

	if (mode & ON_READ) {

		errno = 0;

  		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timer, sizeof(timer)) < 0)
			ERREXIT("Cannot set socket read timeout: %s", strerror(errno));

	}

	if (mode & ON_WRITE) {

		errno = 0;

  		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const void *)&timer, sizeof(timer)) < 0)
      		ERREXIT("Cannot set socket write timeout: %s", strerror(errno));

	}
}

/* SOCKET END-POINTS */

struct sockaddr_in getSocketLocal(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;

	if (getsockname(sock, (struct sockaddr *)&addr, &socksize) == -1)
		ERREXIT("Cannot get socket local address: %s", strerror(errno));

	return addr;
}

struct sockaddr_in getSocketPeer(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;

	if (getpeername(sock, (struct sockaddr *)&addr, &socksize) == -1)
		ERREXIT("Cannot get socket peer address: %s", strerror(errno));

	return addr;
}
