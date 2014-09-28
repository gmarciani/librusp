#include "rudp.h"

static int _RUDP_DEBUG = 1;

/* CONNECTION */

int rudpListen(const int lport) {
	struct sockaddr_in laddr;
	int lsock;	

	laddr = createAddress("127.0.0.1", lport);

	lsock = openSocket();	

	setSocketReusable(lsock);

	bindSocket(lsock, &laddr);

	return lsock;
}

int rudpConnect(const char *ip, const int port) {
	struct sockaddr_in lsaddr;
	int connid;

	lsaddr = createAddress(ip, port);

	connid = synchronizeConnection(lsaddr);

	return connid;
}

int rudpAccept(const int lsock) {
	int connid;

	connid = acceptSynchonization(lsock);

	return connid;
}

void rudpDisconnect(const int connid) {
	if (getConnectionStatus(connid) != _RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot disconnect connection: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	desynchronizeConnection(connid);
}

/* COMMUNICATION */

void rudpSend(const int connid, const char *msg) {
	if (getConnectionStatus(connid) != _RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot send message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	writeOutboxMessage(connid, msg);
}

char *rudpReceive(const int connid, const size_t size) {
	char *msg = NULL;

	if (getConnectionStatus(connid) != _RUDP_CONN_ESTABLISHED) {
		fprintf(stderr, "Cannot receive message: connection not established.\n");
		exit(EXIT_FAILURE);
	}

	msg = readInboxMessage(connid, size);

	return msg;
}

/* SETTINGS */

void setRUDPDebugMode(const int mode) {
	_RUDP_DEBUG = mode;
}
