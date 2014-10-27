#include "rudp.h"

/* CONNECTION */

ConnectionId rudpListen(const int lport) {
	ConnectionId connid;
	Connection *conn = NULL;
	struct sockaddr_in laddr;

	laddr = createAddress(NULL, lport);

	conn = createConnection();

	setListeningConnection(conn, laddr);

	connid = conn->connid;

	return connid;
}

ConnectionId rudpAccept(const ConnectionId lconnid) {
	Connection *lconn = NULL;
	ConnectionId aconnid;

	if (!(lconn = getConnectionById(lconnid)))
		ERREXIT("Cannot retrieve connection: %lld.", lconnid);

	aconnid = passiveOpen(lconn);

	return aconnid;
}

ConnectionId rudpConnect(const char *ip, const int port) {
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

void rudpClose(const ConnectionId connid) {
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	switch (getConnectionState(conn)) {

	case RUDP_CON_LISTEN:
		setConnectionState(conn, RUDP_CON_CLOSED);
		destroyConnection(conn);
		break;

	case RUDP_CON_ESTABL:
		activeClose(conn);
		break;

	case RUDP_CON_CLOSWT:
		passiveClose(conn);
		break;

	case RUDP_CON_CLOSED:
		destroyConnection(conn);
		break;

	default:
		ERREXIT("Cannot handle connection close: %lld.", connid);

	}
}

/* COMMUNICATION */

ssize_t rudpSend(const ConnectionId connid, const char *msg, const size_t size) {
	Connection *conn = NULL;
	ssize_t snd;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	if (getConnectionState(conn) != RUDP_CON_ESTABL)
		return 0;

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_lock(&(conn->sndusrbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (BUFFSIZE - getStrBuffSize(&(conn->sndusrbuff)) < size) {

		if (getConnectionState(conn) != RUDP_CON_ESTABL) {

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

	snd = writeStrBuff(&(conn->sndusrbuff), msg, size);

	if (pthread_mutex_lock(&(conn->sndsgmbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSize(&(conn->sndusrbuff)) > 0 || getSgmBuffSize(&(conn->sndsgmbuff)) > 0) {

		if (getConnectionState(conn) != RUDP_CON_ESTABL) {

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

ssize_t rudpReceive(const ConnectionId connid, char *msg, const size_t size) {
	Connection *conn = NULL;
	ssize_t rcvd;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	if (getConnectionState(conn) != RUDP_CON_ESTABL) {

		memset(msg, 0, sizeof(char) * size);

		return 0;
	}

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_lock(&(conn->rcvusrbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSizeUsr(&(conn->rcvusrbuff)) == 0) {

		if (getConnectionState(conn) != RUDP_CON_ESTABL) {

			if (pthread_mutex_unlock(&(conn->rcvusrbuff.mtx)) > 0)
				ERREXIT("Cannot unlock mutex.");

			memset(msg, 0, sizeof(char) * size);

			return 0;
		}

		if ((error = pthread_cond_timedwait(&(conn->rcvusrbuff.insert_cnd), &(conn->rcvusrbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");
	}

	if (pthread_mutex_unlock(&(conn->rcvusrbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	rcvd = readStrBuff(&(conn->rcvusrbuff), msg, MIN(size, getStrBuffSizeUsr(&(conn->rcvusrbuff))));

	return rcvd;
}

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	addr = getSocketLocal(conn->sock.fd);

	return addr;
}

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection: %lld.", connid);

	addr = getSocketPeer(conn->sock.fd);

	return addr;
}
