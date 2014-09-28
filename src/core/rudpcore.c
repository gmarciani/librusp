#include "rudpcore.h"

static int _RUDP_CORE_DEBUG = 1;

static Connection *_RUDP_CONNECTIONS_POOL[_RUDP_MAX_CONNECTIONS];
static int _RUDP_CONNECTIONS_INPOOL = 0;
static int _RUDP_NEXT_CONNECTION_ID = 0;

/* CONNECTION */

int synchronizeConnection(const struct sockaddr_in laddr) {
	Connection *conn = NULL;
	Segment sgm, syn, synack, acksynack;

	conn = _createConnection();

	conn->record.paddr = laddr;

	conn->record.outbox = createOutbox(_getISN(), _RUDP_WND);

	syn = createSegment(_RUDP_SYN, 0, 0, NULL);

	sendSegment(conn, syn);

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

int acceptSynchonization(const int lsock) {
	Connection *conn = NULL;
	Segment sgm, syn, synack, acksynack;
	struct sockaddr_in caddr;
	char *ssgm = NULL;

	conn = _createConnection();

	do {
		ssgm = readUnconnectedSocket(lsock, &caddr, _RUDP_MAX_SGM);
		sgm = deserializeSegment(ssgm);
		free(ssgm);
	} while (sgm.hdr.ctrl != _RUDP_ACK);

	syn = sgm;	

	conn->record.paddr = caddr;

	conn->record.inbox = createInbox(syn.hdr.seqno, _RUDP_WND);	

	conn->record.status = _RUDP_CONN_SYN_RCVD;

	conn->record.outbox = createOutbox(_getISN(), _RUDP_WND);

	synack = createSegment(_RUDP_ACK, 0, 0, NULL);	

	sendSegment(conn, synack);

	conn->record.status = _RUDP_CONN_SYN_SENT;

	// receive acksynack

	return conn->connid;
}

void desynchronizeConnection(const int connid) {
	// to implement!
}

Connection *_createConnection(void) {
	Connection *conn = NULL;

	if (_RUDP_CONNECTIONS_INPOOL == _RUDP_MAX_CONNECTIONS) {
		fprintf(stderr, "Cannot allocate connection: too many connections in pool.\n");
		return NULL;
	}

	if (!(conn = malloc(sizeof(Connection)))) {
		fprintf(stderr, "Cannot allocate new connection.\n");
		exit(EXIT_FAILURE);
	}

	conn->connid = _RUDP_NEXT_CONNECTION_ID;
	_RUDP_NEXT_CONNECTION_ID++;

	_RUDP_CONNECTIONS_POOL[_RUDP_CONNECTIONS_INPOOL] = conn;
	_RUDP_CONNECTIONS_INPOOL++;

	conn->version = _RUDP_VERSION;	
	conn->record.status = _RUDP_CONN_CLOSED;
	conn->record.sock = openSocket();	

	return conn;
}
	
void _destroyConnection(Connection *conn) {
	// to implement!
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(const int connid, const char *msg) {
	// to implement!
}

char *readInboxMessage(const int connid, const size_t size) {
	char *msg = NULL;
	
	// to implement!	
	
	return msg;
}

/* SEGMENT COMMUNICATION */

void sendSegment(Connection *conn, Segment sgm) {
	if (conn->record.status == _RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot send segment: connection closed.\n");
		exit(EXIT_FAILURE);
	}

	submitSegmentToOutbox(&(conn->record.outbox), sgm);

	flushOutbox(conn);
}

void flushOutbox(Connection *conn) {
	OutboxElement *curr = NULL;
	char *ssgm = NULL;	

	while (conn->record.outbox.size != 0) {
		curr = conn->record.outbox.wndbase;
		while (curr) {
			if (conn->record.outbox.wndend)
				if (curr == conn->record.outbox.wndend->next)
					break;
			if (curr->status == _RUDP_UNACKED) {
				ssgm = serializeSegment(*(curr->segment));
				writeConnectedSocket(conn->record.sock, ssgm);
				if (_RUDP_CORE_DEBUG)
					printOutSegment(conn->record.paddr, *(curr->segment));
				free(ssgm);
				// start timeout
			}
			curr = curr->next;
		}
	}	
}

Segment receiveSegment(Connection *conn) {	
	Segment sgm;

	sgm = readInboxSegment(&(conn->record.inbox)); //il primo segmento affidabile

	return sgm;
}

/* UTILITY */

int getConnectionStatus(const int connid) {
	Connection *conn;
	int status;

	conn = _getConnectionById(connid);

	status = conn->record.status;

	return status;
}

Connection *_getConnectionById(const int connid) {
	int i;

	for (i = 0; i < _RUDP_CONNECTIONS_INPOOL; i++)
		if (_RUDP_CONNECTIONS_POOL[i]->connid == connid)
			return _RUDP_CONNECTIONS_POOL[i];

	return NULL;
}

unsigned long int _getISN() {
	unsigned long int isn;
	struct timeval time;

	gettimeofday(&time, NULL);
	isn = (1000000 * time.tv_sec + time.tv_usec) % 4;

	return isn;
}

/* SETTING */

void setRUDPCoreDebugMode(const int mode) {
	_RUDP_CORE_DEBUG = mode;
}
