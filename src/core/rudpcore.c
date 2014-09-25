#include "rudpcore.h"

static int RUDP_DEBUG = 1;

/* CONNECTION */

Connection createConnection() {
	Connection conn;

	conn.version = _RUDP_VERSION;
	conn.status = _RUDP_CONN_CLOSED;
	conn.sock = openSocket();	
	conn.seqno = getISN();
	conn.ackno = 0;
	conn.wnd = _RUDP_WND;

	return conn;
}

Connection synchronizeConnection(const struct sockaddr_in laddr) {
	Connection conn;

	return conn;
}

Connection acceptSynchonization(const int lsock) {
	Connection conn;

	return conn;
}

void desynchronizeConnection(Connection *conn) {

}
	
void destroyConnection(Connection *conn) {

}

unsigned long int getISN() {
	unsigned long int isn;
	struct timeval time;

	gettimeofday(&time, NULL);
	isn = (1000000 * time.tv_sec + time.tv_usec) % 4;

	return isn;
}

void sendStream(Connection *conn, const Stream stream) {

}

char *receiveStream(Connection *conn, const size_t size) {
	char* msg = NULL;

	return msg;
}

/* COMMUNICATION */

void sendSegment(Connection *conn, Segment *sgm) {
	char *ssgm;

	sgm->hdr.seqno = conn->seqno;
	sgm->hdr.ackno = conn->lastack;

	ssgm = serializeSegment(*sgm);

	if (write(conn->sock, ssgm, _RUDP_MAX_SGM) == -1) {
		fprintf(stderr, "Error in socket write.\n");
		exit(EXIT_FAILURE);
	}

	free(ssgm);
}

Segment receiveSegment(Connection *conn) {	
	Segment sgm;
	char *ssgm;
	ssize_t rcvd;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation for packet receive.\n");
		exit(EXIT_FAILURE);
	}

	rcvd = 0;
	if ((rcvd = read(conn->sock, ssgm, _RUDP_MAX_SGM)) == -1) {
		fprintf(stderr, "Error in socket read.\n");
		exit(EXIT_FAILURE);
	}

	ssgm[rcvd] = '\0';

	sgm = deserializeSegment(ssgm);

	free(ssgm);

	return sgm;
}

/* SETTING */

void setDebugMode(const int mode) {
	RUDP_DEBUG = mode;
}
