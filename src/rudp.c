#include "rudp.h"

/* CONNECTION */

ConnectionId rudpListen(const int lport) {
	ConnectionId connid;
	Connection *conn = NULL;
	struct sockaddr_in laddr;

	laddr = createAddress("127.0.0.1", lport);

	conn = createConnection();

	if (!conn)
		return -1;

	setListeningConnection(conn, laddr);

	connid = conn->connid;

	return connid;
}

ConnectionId rudpAccept(const ConnectionId lconnid) {
	Connection *lconn = NULL;
	ConnectionId aconnid;

	lconn = getConnectionById(lconnid);

	if (!lconn)
		ERREXIT("Cannot retrieve connection.");

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
		ERREXIT("Cannot retrieve connection.");	

	desynchronizeConnection(conn);
}

void rudpClose(const ConnectionId connid) {	
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection.");	

	destroyConnection(conn);
}

/* COMMUNICATION */

void rudpSend(const ConnectionId connid, const char *msg, const size_t size) {
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection.");	

	writeMessage(conn, msg, size);
}

char *rudpReceive(const ConnectionId connid, const size_t size) {
	Connection *conn = NULL;	
	char *msg = NULL;	

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection.");

	msg = readMessage(conn, size);

	return msg;
}

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection.");

	addr = getSocketLocal(conn->sock);

	return addr;
}

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn)
		ERREXIT("Cannot retrieve connection.");

	addr = getSocketPeer(conn->sock);

	return addr;
}
