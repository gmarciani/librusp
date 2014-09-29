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

	if (!lconn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", lconnid);
		exit(EXIT_FAILURE);
	}	

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

	if (!conn)
		return -1;

	syncresult = synchronizeConnection(conn, laddr);

	if (syncresult == -1)
		return -1;		

	connid = conn->connid;

	return connid;
}

void rudpDisconnect(const ConnectionId connid) {
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.state != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot disconnect connection: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	desynchronizeConnection(conn);
}

void rudpClose(const ConnectionId connid) {	
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.state != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot close connection: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	destroyConnection(conn);
}

/* COMMUNICATION */

void rudpSend(const ConnectionId connid, const char *msg) {
	Connection *conn = NULL;

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}		

	if (conn->record.state != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot send message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	writeOutboxMessage(conn, msg);
}

char *rudpReceive(const ConnectionId connid, const size_t size) {
	Connection *conn = NULL;	
	char *msg = NULL;	

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}	

	if (conn->record.state != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot receive message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	msg = readInboxMessage(conn, size);

	return msg;
}

/* UTILITY */

struct sockaddr_in rudpGetLocalAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}

	if (conn->record.state == RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot get local address: connection closed.\n");
		exit(EXIT_FAILURE);
	}

	addr = getSocketLocal(conn->record.sock);

	return addr;
}

struct sockaddr_in rudpGetPeerAddress(const ConnectionId connid) {
	Connection *conn = NULL;
	struct sockaddr_in addr;

	conn = getConnectionById(connid);

	if (!conn) {
		fprintf(stderr, "Cannot retrieve connection with id: %d.\n", connid);
		exit(EXIT_FAILURE);
	}

	if (conn->record.state != RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot get peer address: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	addr = getSocketPeer(conn->record.sock);

	return addr;
}

char *rudpAddressToString(const struct sockaddr_in addr) {
	char *straddr = NULL;
	char *strip = NULL;
	int port;

	strip = getIp(addr);

	port = getPort(addr);

	if (!(straddr = malloc(sizeof(char) * (ADDRESS_IPV4_MAX_OUTPUT + 1)))) {
		fprintf(stderr, "Cannot allocate memory for address to string.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(straddr, "%s:%d", strip, port);

	free(strip);

	return straddr;

}
