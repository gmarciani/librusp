#include "rudpcore.h"

static int RUDP_CORE_DEBUG = 1;

static Connection *RUDP_CONNECTIONS_POOL[RUDP_MAX_CONNECTIONS];
static int RUDP_CONNECTIONS_POOL_SIZE = 0;
static int RUDP_NEXT_CONNECTION_ID = 0;

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (RUDP_CONNECTIONS_POOL_SIZE == RUDP_MAX_CONNECTIONS) {
		fprintf(stderr, "Cannot allocate connection: too many connections in pool.\n");
		return NULL;
	}

	if (!(conn = malloc(sizeof(Connection)))) {
		fprintf(stderr, "Cannot allocate new connection.\n");
		exit(EXIT_FAILURE);
	}

	conn->connid = RUDP_NEXT_CONNECTION_ID;
	RUDP_NEXT_CONNECTION_ID++;

	RUDP_CONNECTIONS_POOL[RUDP_CONNECTIONS_POOL_SIZE] = conn;
	RUDP_CONNECTIONS_POOL_SIZE++;

	conn->version = RUDP_VERSION;	
	conn->record.status = RUDP_CONN_CLOSED;
	conn->record.sock = openSocket();	

	return conn;
}

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {
	setSocketReusable(conn->record.sock);	
	bindSocket(conn->record.sock, &laddr);
	conn->record.inbox = createInbox(RUDP_MAX_WNDS);
	//setListeningInbox(conn->record.inbox);
	conn->record.status = RUDP_CONN_LISTEN;	
}

ConnectionId acceptSynchonization(Connection *lconn) {
	Connection *aconn = NULL;
	Segment syn, synack, acksynack;
	struct sockaddr_in caddr;
	char *ssgm = NULL;	

	if (lconn->record.status != RUDP_CONN_LISTEN) {
		fprintf(stderr, "Cannot accept connection: connection is not listening.\n");
		exit(EXIT_FAILURE);
	}

	/*syn = receiveSegment(lconn);	

	conn->record.paddr = caddr;

	conn->record.inbox = createInbox(syn.hdr.seqno, _RUDP_WND);	

	conn->record.status = _RUDP_CONN_SYN_RCVD;

	conn->record.outbox = createOutbox(_getISN(), _RUDP_WND);

	synack = createSegment(_RUDP_ACK, 0, 0, NULL);	

	sendSegment(conn, synack);

	conn->record.status = _RUDP_CONN_SYN_SENT;

	// receive acksynack*/

	return aconn->connid;
}

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr) {
	Segment sgm, syn, synack, acksynack;

	conn->record.paddr = laddr;

	conn->record.outbox = createOutbox(getISN(), RUDP_MAX_WNDS);

	syn = createSegment(RUDP_SYN, 0, 0, 0, 0, NULL);

	sendSegment(conn, syn);

	do {
		
		sgm = _receiveSegment(conn);
	while (sgm.hdr.ctrl != (RUDP_SYN | RUDP_ACK));

	conn->record.status = _RUDP_CONN_SYN_SENT;

	// receive synack

	conn->record.status = _RUDP_CONN_SYN_RCVD;

	conn->record.inbox = createInbox(synack.hdr.seqno, _RUDP_WND);

	acksynack = createSegment(_RUDP_ACK, 0, 0, NULL); // con quale ackno?

	sendSegment(conn, acksynack);

	setSocketConnected(conn->record.sock, conn->record.paddr);

	conn->record.status = _RUDP_CONN_ESTABLISHED;

	return conn->connid;
}

void desynchronizeConnection(Connection *conn) {
	Segment fin, finack;

	conn->record.status = RUDP_WAITCLOSE; // posso inviare solo FIN, e ricevere solo FINACK

	fin = createSegment(RUDP_FIN, 0, 0, 0, 0, NULL);

	sendSegment(conn, fin);

	finack = receiveSegment(conn);

	destroyConnection(conn);
}


	
void destroyConnection(Connection *conn) {
	conn->record.status = RUDP_CONN_CLOSED;

	freeOutbox(conn->record.outbox);

	freeInbox(conn->record.inbox);

	if (close(conn->record.sock) == -1) {
		fprintf("Cannot close socket.\n");
		exit(EXIT_FAILURE);
	}

	free(conn);

	RUDP_CONNECTION_POOL_SIZE--;	
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg) {
	Stream *stream = NULL;
	int i;

	stream = createStream(msg);

	for (i = 0; i < stream.size; i++)
		sendSegment(conn, sgm);
}

char *readInboxMessage(Connection *conn, const size_t size) {
	char *msg = NULL;
	
	msg = readInboxBuffer(conn->record.inbox, size);	
	
	return msg;
}

/* SEGMENT COMMUNICATION */

void sendSegment(Connection *conn, Segment sgm) {
	if (conn->record.status == RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot send segment: connection closed.\n");
		exit(EXIT_FAILURE);
	}

	submitSegmentToOutbox(&(conn->record.outbox), sgm);
}

Segment receiveSegment(Connection *conn) {	
	Segment sgm;

	sgm = readInboxSegment(&(conn->record.inbox));

	return sgm;
}

/* UTILITY */

Connection *getConnectionById(const int connid) {
	int i;

	for (i = 0; i < RUDP_CONNECTIONS_POOL_SIZE; i++)
		if (RUDP_CONNECTIONS_POOL[i]->connid == connid)
			return RUDP_CONNECTIONS_POOL[i];

	return NULL;
}

uint32_t getISN() {
	uint32_t isn;
	struct timeval time;

	gettimeofday(&time, NULL);
	isn = (uint32_t) (1000000 * time.tv_sec + time.tv_usec) % 4;

	return isn;
}

/* SETTING */

void setRUDPCoreDebugMode(const int mode) {
	RUDP_CORE_DEBUG = mode;
}
