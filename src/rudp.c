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
		ERREXIT("Cannot retrieve connection: %ld", lconnid);

	aconnid = acceptSynchonization(lconn);

	return aconnid;
}

ConnectionId rudpConnect(const char *ip, const int port) {
	ConnectionId connid;
	Connection *conn = NULL;
	struct sockaddr_in laddr;
	int syncresult;

	laddr = createAddress(ip, port);

	conn = createConnection();

	syncresult = synchronizeConnection(conn, laddr);

	if (syncresult == -1) {

		destroyConnection(conn);

		return -1;
	}

	connid = conn->connid;

	return connid;
}

void rudpDisconnect(const ConnectionId connid) {
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection: %ld", connid);	

	desynchronizeConnection(conn);
}

void rudpClose(const ConnectionId connid) {	
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %ld", connid);

	destroyConnection(conn);
}

/* COMMUNICATION */

void rudpSend(const ConnectionId connid, const char *msg, const size_t size) {
	Connection *conn = NULL;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %ld", connid);		

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot write message: connection not established.");

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_lock(&(conn->sndbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (BUFFSIZE - getStrBuffSize(&(conn->sndbuff)) < size)
		if ((error = pthread_cond_timedwait(&(conn->sndbuff.remove_cnd), &(conn->sndbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");

	if (pthread_mutex_unlock(&(conn->sndbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	writeStrBuff(&(conn->sndbuff), msg, size);

	if (pthread_mutex_lock(&(conn->sndsgmbuff.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getStrBuffSize(&(conn->sndbuff)) > 0 || getSgmBuffSize(&(conn->sndsgmbuff)) > 0)
		if ((error = pthread_cond_timedwait(&(conn->sndsgmbuff.remove_cnd), &(conn->sndsgmbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");

	if (pthread_mutex_unlock(&(conn->sndsgmbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");
}

size_t rudpReceive(const ConnectionId connid, char *msg, const size_t size) {
	Connection *conn = NULL;
	size_t rcvd;
	struct timespec timeout;
	int error;

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %ld", connid);	

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot read message: connection not established.");

	timeout = getTimespec(conn->timeout.value);

	if (pthread_mutex_unlock(&(conn->rcvbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	while (conn->rcvbuff.size < size)
		if ((error = pthread_cond_timedwait(&(conn->rcvbuff.insert_cnd), &(conn->rcvbuff.mtx), &timeout)) > 0)
			if (error != ETIMEDOUT)
				ERREXIT("Cannot timed wait condition variable.");

	if (pthread_mutex_unlock(&(conn->rcvbuff.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	rcvd = readStrBuff(&(conn->rcvbuff), msg, size);

	return rcvd;
}

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection: %ld", connid);

	addr = getSocketLocal(conn->sock.fd);

	return addr;
}

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection: %ld", connid);

	addr = getSocketPeer(conn->sock.fd);

	return addr;
}
