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


	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %ld", connid);		

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot write message: connection not established.");

	writeStrBuff(conn->sndbuff, msg, size);
}

char *rudpReceive(const ConnectionId connid, const size_t size) {
	Connection *conn = NULL;
	struct timespec timeout;
	char *msg = NULL;	

	if (!(conn = getConnectionById(connid)))
		ERREXIT("Cannot retrieve connection: %ld", connid);	

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot read message: connection not established.");

	timeout = getTimespec(conn->timeout->value);

	lockMutex(conn->rcvbuff->mtx);

	while (conn->rcvbuff->size < size)
		waitTimeoutConditionVariable(conn->rcvbuff->insert_cnd, conn->rcvbuff->mtx, timeout);

	msg = readStrBuff(conn->rcvbuff, size);

	unlockMutex(conn->rcvbuff->mtx);

	return msg;
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
