#include "rudp.h"

static int RUDP_DEBUG = 1;

/* CONNECTION */

int rudpListen(const int lport) {
	struct sockaddr_in laddr;
	int lsock;	

	laddr = _rudpAddress("127.0.0.1", lport);

	lsock = _rudpOpenSocket();	

	_rudpReusableSocket(lsock);

	_rudpBindSocket(lsock, &laddr);

	return lsock;
}

rudpconn_t rudpConnect(const char *ip, const int port) {
	struct sockaddr_in lsaddr;
	rudpconn_t conn;

	lsaddr = _rudpAddress(ip, port);

	conn = _rudpASYNHandshake(lsaddr);

	return conn;
}

rudpconn_t rudpAccept(const int lsock) {
	rudpconn_t conn;
	
	conn = _rudpPSYNHandshake(lsock)

	return conn;
}

void rudpDisconnect(rudpConn_t *conn) {
	_rudpFINHanshake(conn);
}

/* COMMUNICATION */

void rudpSend(const rudpConn_t *conn, const char *msg) {
	rudpstream_t stream;

	stream = _rudpSegmentStream(msg);

	_rudpSendStream(conn, stream);
}

char *rudpReceive(const rudpConn_t *conn) {
	_rudpReceiveStream(conn);
}
