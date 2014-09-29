#include "rudp.h"

static int RUDP_DEBUG = 1;

/* CONNECTION */

ConnectionId rudpListen(const int lport) {
	Connection *conn;
	struct sockaddr_in laddr;

	laddr = createAddress("127.0.0.1", lport);

	conn = createConnection();

	setListeningConnection(conn, laddr);

	return conn->connid;
}

ConnectionId rudpConnect(const char *ip, const int port) {
	Connection *conn;
	struct sockaddr_in laddr;

	saddr = createAddress(ip, port);

	conn = createConnection();

	if (synchronizeConnection(conn, laddr) == -1)
		return -1;
	else
		return conn->connid;
}

ConnectionId rudpAccept(const ConnectionId lconnid) {
	Connection *lconn = NULL;
	ConnectionId aconnid;

	if (!(lconn = getConnectionById(lconnid))) {
		fprintf(stderr, "Cannot retrieve connection with specified id: %d.\n", lconnid);
		exit(EXIT_FAILURE);
	}	

	aconnid = acceptSynchonization(lconn);

	return aconnid;
}

void rudpDisconnect(const ConnectionId connid) {
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid))) {
		fprintf(stderr, "Cannot retrieve connection with specified id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.status != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot disconnect connection: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	desynchronizeConnection(conn);
}

void rudpClose(const ConnectionId connid) {	
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid))) {
		fprintf(stderr, "Cannot retrieve connection with specified id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.status == RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot close connection: connection already closed.\n");
		exit(EXIT_FAILURE);
	}

	destroyConnection(conn);
}

/* COMMUNICATION */

void rudpSend(const ConnectionId connid, const char *msg) {
	Connection *conn = NULL;

	if (!(conn = getConnectionById(connid))) {
		fprintf(stderr, "Cannot retrieve connection with specified id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.status != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot send message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	writeOutboxMessage(conn, msg);
}

char *rudpReceive(const ConnectionId connid, const size_t size) {
	Connection *conn = NULL;	
	char *msg = NULL;	

	if (!(conn = getConnectionById(connid))) {
		fprintf(stderr, "Cannot retrieve connection with specified id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.status != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot receive message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	msg = readInboxMessage(conn, size);

	return msg;
}

/* SETTINGS */

void setRUDPDebugMode(const int mode) {
	RUDP_DEBUG = mode;
}
