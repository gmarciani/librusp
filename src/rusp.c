#include "rusp.h"

/* CONNECTION */

ConnectionId ruspListen(const int lport) {
	ConnectionId connid;
	Connection *conn = NULL;
	struct sockaddr_in laddr;

	laddr = createAddress(NULL, lport);

	conn = createConnection();

	setListeningConnection(conn, laddr);

	connid = conn->connid;

	return connid;
}

ConnectionId ruspAccept(const ConnectionId lconnid) {
	Connection *lconn = NULL;
	ConnectionId aconnid;

	if (!(lconn = getConnectionById(lconnid)))
		ERREXIT("Cannot retrieve connection: %lld.", lconnid);

	aconnid = passiveOpen(lconn);

	return aconnid;
}

ConnectionId ruspConnect(const char *ip, const int port) {
	ConnectionId connid;
	Connection *conn = NULL;
	struct sockaddr_in laddr;
	int syncresult;

	laddr = createAddress(ip, port);

	conn = createConnection();

	syncresult = activeOpen(conn, laddr);

	if (syncresult == -1) {

		destroyConnection(conn);

		return -1;
	}

	connid = conn->connid;

	return connid;
}

void ruspClose(const ConnectionId connid) {
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	switch (getConnectionState(conn)) {

	case RUSP_CON_LISTEN:
		setConnectionState(conn, RUSP_CON_CLOSED);
		destroyConnection(conn);
		break;

	case RUSP_CON_ESTABL:
		activeClose(conn);
		break;

	case RUSP_CON_CLOSWT:
		passiveClose(conn);
		break;

	case RUSP_CON_CLOSED:
		destroyConnection(conn);
		break;

	default:
		ERREXIT("Cannot handle connection close: %lld.", connid);

	}
}

/* COMMUNICATION */

ssize_t ruspSend(const ConnectionId connid, const char *msg, const size_t msgs) {
	Connection *conn = NULL;
	ssize_t snd;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	if (getConnectionState(conn) != RUSP_CON_ESTABL)
		return 0;

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_lock(&(conn->sndusrbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (BUFFSIZE - getStrBuffSize(&(conn->sndusrbuff)) < msgs) {

		if (getConnectionState(conn) != RUSP_CON_ESTABL) {

			if (pthread_mutex_unlock(&(conn->sndusrbuff.mtx)) > 0)
				ERREXIT("Cannot unlock mutex.");

			return 0;
		}

		if ((error = pthread_cond_timedwait(&(conn->sndusrbuff.remove_cnd), &(conn->sndusrbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");
	}

	if (pthread_mutex_unlock(&(conn->sndusrbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	snd = writeStrBuff(&(conn->sndusrbuff), msg, msgs);

	if (pthread_mutex_lock(&(conn->sndsgmbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSize(&(conn->sndusrbuff)) > 0 || getSgmBuffSize(&(conn->sndsgmbuff)) > 0) {

		if (getConnectionState(conn) != RUSP_CON_ESTABL) {

			if (pthread_mutex_unlock(&(conn->sndusrbuff.mtx)) > 0)
				ERREXIT("Cannot unlock mutex.");

			return 0;
		}

		if ((error = pthread_cond_timedwait(&(conn->sndsgmbuff.remove_cnd), &(conn->sndsgmbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");
	}

	if (pthread_mutex_unlock(&(conn->sndsgmbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	return snd;
}

ssize_t ruspReceive(const ConnectionId connid, char *msg, const size_t msgs) {
	Connection *conn = NULL;
	ssize_t rcvd;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	if (getConnectionState(conn) != RUSP_CON_ESTABL) {

		memset(msg, 0, sizeof(char) * msgs);

		return 0;
	}

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_lock(&(conn->rcvusrbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSizeUsr(&(conn->rcvusrbuff)) == 0) {

		if (getConnectionState(conn) != RUSP_CON_ESTABL) {

			if (pthread_mutex_unlock(&(conn->rcvusrbuff.mtx)) > 0)
				ERREXIT("Cannot unlock mutex.");

			memset(msg, 0, sizeof(char) * msgs);

			return 0;
		}

		if ((error = pthread_cond_timedwait(&(conn->rcvusrbuff.insert_cnd), &(conn->rcvusrbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");
	}

	if (pthread_mutex_unlock(&(conn->rcvusrbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	rcvd = readStrBuff(&(conn->rcvusrbuff), msg, MIN(msgs, getStrBuffSizeUsr(&(conn->rcvusrbuff))));

	return rcvd;
}

/* ADDRESS UTILITY */

int ruspLocal(const ConnectionId connid, struct sockaddr_in *addr) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid)))
		return -1;

	errno = 0;

	if (getsockname(conn->sock.fd, (struct sockaddr *)addr, &socksize) == -1)
		ERREXIT("Cannot get socket local address: %s", strerror(errno));

	return 0;
}

int ruspPeer(const ConnectionId connid, struct sockaddr_in *addr) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid)))
		return -1;

	errno = 0;

	if (getpeername(conn->sock.fd, (struct sockaddr *)addr, &socksize) == -1)
		ERREXIT("Cannot get socket peer address: %s", strerror(errno));

	return 0;
}

/* DEV UTILITY */

double rudpGetDrop(void) {
	return RUSP_DROP;
}

void rudpSetDrop(const double drop) {
	RUSP_DROP = drop;
}

int rudpGetDebug(void) {
	return RUSP_DEBUG;
}

void rudpSetDebug(const int debug) {
	RUSP_DEBUG = debug;
}
