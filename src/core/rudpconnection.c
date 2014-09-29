#include "rudpconnection.h"

static Connection *RUDP_CONNECTIONS_POOL[RUDP_MAX_CONNECTIONS];

static int RUDP_CONNECTIONS_POOL_SIZE = 0;

static int RUDP_NEXT_CONNECTION_ID = 0;

static void setConnectionState(Connection *conn, unsigned short state);

static void sendSegment(Connection *conn, Segment sgm);

//static Segment receiveSegment(Connection *conn);

static uint32_t getISN();

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

	conn->version = RUDP_VERSION;

	conn->connid = RUDP_NEXT_CONNECTION_ID;

	RUDP_NEXT_CONNECTION_ID++;

	setConnectionState(conn, RUDP_CONN_CLOSED);

	RUDP_CONNECTIONS_POOL[RUDP_CONNECTIONS_POOL_SIZE] = conn;

	RUDP_CONNECTIONS_POOL_SIZE++;	

	printf("Connection created with id: %d\n", conn->connid);

	return conn;
}

Connection *getConnectionById(const int connid) {
	int i;

	for (i = 0; i < RUDP_CONNECTIONS_POOL_SIZE; i++) {
		if (RUDP_CONNECTIONS_POOL[i]) {
			if (RUDP_CONNECTIONS_POOL[i]->connid == connid)
				return RUDP_CONNECTIONS_POOL[i];
		}		
	}		

	return NULL;
}

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {
	if (conn->record.state != RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot make connection listening: connection not closed.\n");
		exit(EXIT_FAILURE);
	}

	conn->record.sock = openSocket();

	setSocketReusable(conn->record.sock);
	
	bindSocket(conn->record.sock, &laddr);

	setConnectionState(conn, RUDP_CONN_LISTEN);
}

ConnectionId acceptSynchonization(Connection *lconn) {
	Connection *aconn = NULL;
	struct sockaddr_in caddr;	
	Segment syn, synack, acksynack;
	int asock;
	char *lssgm = NULL;
	char *pssgm = NULL;	
	uint32_t pisn;
	uint32_t lisn;	

	if (lconn->record.state != RUDP_CONN_LISTEN) {
		fprintf(stderr, "Cannot accept connection: connection not listening.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {

		do {
			pssgm = readUnconnectedSocket(lconn->record.sock, &caddr, RUDP_MAX_SGM);			

			syn = deserializeSegment(pssgm);
			
			printInSegment(caddr, syn);

			free(pssgm);

		} while (syn.hdr.ctrl != RUDP_SYN);

		setConnectionState(lconn, RUDP_CONN_SYN_RCVD);

		pisn = syn.hdr.seqn;

		lisn = getISN();

		asock = openSocket();

		setSocketConnected(asock, caddr);	

		setSocketReadTimeout(asock, RUDP_ACK_TIMEOUT);

		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, lisn, ((pisn + 1) % RUDP_MAX_SEQN), NULL);	

		lssgm = serializeSegment(synack);

		writeConnectedSocket(asock, lssgm);		

		printOutSegment(caddr, synack);

		free(lssgm);	

		setConnectionState(lconn, RUDP_CONN_SYN_SENT);	

		pssgm = readConnectedSocket(asock, RUDP_MAX_SGM);

		if (pssgm) {

			acksynack = deserializeSegment(pssgm);

			printInSegment(caddr, acksynack);

			free(pssgm);

			if ((acksynack.hdr.ctrl == RUDP_ACK) &&
				(acksynack.hdr.seqn == ((pisn + 1) % RUDP_MAX_SEQN)) &&
				(acksynack.hdr.ackn == ((lisn + 1) % RUDP_MAX_SEQN))) {
			
				aconn = createConnection();	

				if (aconn) {
					aconn->record.outbox = createOutbox(((lisn + 1) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

					aconn->record.inbox = createInbox(((pisn + 2) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

					aconn->record.sock = asock;

					setConnectionState(aconn, RUDP_CONN_ESTABLISHED);

					return aconn->connid;
				}				
			}
		}		

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CONN_LISTEN);		
	}
}

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr) {
	struct sockaddr_in aaddr;	
	Segment syn, synack, acksynack;
	int asock;
	char *lssgm = NULL;
	char *pssgm = NULL;	
	uint32_t lisn;
	uint32_t pisn;	

	if (conn->record.state != RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot synchronize connection: connection already active.\n");
		exit(EXIT_FAILURE);
	}

	lisn = getISN();

	asock = openSocket();

	setSocketReadTimeout(asock, RUDP_ACK_TIMEOUT);	

	syn = createSegment(RUDP_SYN, 0, 0, lisn, 0, NULL);

	lssgm = serializeSegment(syn);

	writeUnconnectedSocket(asock, laddr, lssgm);

	printOutSegment(laddr, syn);

	free(lssgm);

	setConnectionState(conn, RUDP_CONN_SYN_SENT);

	pssgm = readUnconnectedSocket(asock, &aaddr, RUDP_MAX_SGM);

	if (pssgm) {

		synack = deserializeSegment(pssgm);

		printInSegment(aaddr, synack);

		free(pssgm);

		if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &&
			(synack.hdr.ackn == ((lisn + 1) % RUDP_MAX_SEQN))) {

			setConnectionState(conn, RUDP_CONN_SYN_RCVD);

			setSocketConnected(asock, aaddr);

			pisn = synack.hdr.seqn;

			acksynack = createSegment(RUDP_ACK, 0, 0, ((lisn + 1) % RUDP_MAX_SEQN), ((pisn + 1) % RUDP_MAX_SEQN), NULL);

			lssgm = serializeSegment(acksynack);

			writeConnectedSocket(asock, lssgm);

			printOutSegment(aaddr, acksynack);

			free(lssgm);

			conn->record.outbox = createOutbox(((lisn + 2) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

			conn->record.inbox = createInbox(((pisn + 1) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

			conn->record.sock = asock;
			
			setConnectionState(conn, RUDP_CONN_ESTABLISHED);

			return conn->connid;					
		}
	}	

	closeSocket(asock);

	setConnectionState(conn, RUDP_CONN_CLOSED);

	return -1;
}

void desynchronizeConnection(Connection *conn) {
	//four-way handshake
	destroyConnection(conn);
}
	
void destroyConnection(Connection *conn) {
	conn->record.state = RUDP_CONN_CLOSED;

	freeInbox(conn->record.inbox);

	freeOutbox(conn->record.outbox);	

	if (close(conn->record.sock) == -1) {
		fprintf(stderr, "Cannot close socket.\n");
		exit(EXIT_FAILURE);
	}

	free(conn);

	RUDP_CONNECTIONS_POOL_SIZE--;	
}

static void setConnectionState(Connection *conn, unsigned short state) {
	conn->record.state = state;
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg) {
	Stream *stream = NULL;
	int i;

	stream = createStream(msg);

	for (i = 0; i < stream->size; i++)
		sendSegment(conn, stream->segments[i]);
}

char *readInboxMessage(Connection *conn, const size_t size) {
	char *msg = NULL;
	
	msg = readInboxBuffer(conn->record.inbox, size);	
	
	return msg;
}

/* SEGMENT COMMUNICATION */

static void sendSegment(Connection *conn, Segment sgm) {

	submitSegmentToOutbox(conn->record.outbox, sgm);
}
/*
static Segment receiveSegment(Connection *conn) {	
	Segment sgm;

	sgm = readInboxSegment(conn->record.inbox);

	return sgm;
}*/

/* UTILITY */

static uint32_t getISN() {
	uint32_t isn;
	struct timeval time;

	gettimeofday(&time, NULL);
	isn = (uint32_t) (((1000 * time.tv_sec + time.tv_usec) % 4) % RUDP_MAX_SEQN);

	printf("Getting ISN %u\n", isn);

	return isn;
}
