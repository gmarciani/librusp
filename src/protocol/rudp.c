#include "rudp.h"

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

Connection rudpConnect(const char *ip, const int port) {
	struct sockaddr_in lsaddr;
	Connection conn;

	lsaddr = createAddress(ip, port);

	conn = synchronizeConnection(lsaddr);

	return conn;
}

Connection rudpAccept(const int lsock) {
	Connection conn;

	conn = acceptSynchonization(lsock);

	return conn;
}

void rudpDisconnect(Connection *conn) {
	desynchronizeConnection(conn);
	
	destroyConnection(conn);
}

/* COMMUNICATION */

void rudpSend(Connection *conn, const char *msg) {
	Stream stream;

	stream = createStream(msg);

	sendStream(conn, stream);
}

char *rudpReceive(Connection *conn, const size_t size) {
	char *msg;

	msg = receiveStream(conn, size);

	return msg;
}
