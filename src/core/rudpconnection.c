#include "rudpconnection.h"

static int RUDP_CONN_DEBUG = 1;

/* CONNECTIONS POOL */

static List CONNPOOL = LIST_INITIALIZER;

static int CONNPOOL_NEXTID = 0;

static pthread_mutex_t connpool_mtx = PTHREAD_MUTEX_INITIALIZER;

/* CONNECTION */

static void *managerLoop(void *arg);

static void timeoutHandler(union sigval arg);

static uint32_t getISN(void);

Connection *createConnection(void) {
	Connection *conn = NULL;

	lockMutex(&connpool_mtx);

	if (!(conn = malloc(sizeof(Connection))) ||
		!(conn->record = malloc(sizeof(ConnectionRecord))) ||
		!(conn->record->conn_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->record->conn_cnd = malloc(sizeof(pthread_cond_t)))) {

		fprintf(stderr, "Cannot allocate memory for connection resources.\n");

		exit(EXIT_FAILURE);
	}

	conn->version = RUDP_VERSION;

	conn->connid = CONNPOOL_NEXTID;

	initializeMutex(conn->record->conn_mtx);

	initializeConditionVariable(conn->record->conn_cnd);

	conn->record->state = RUDP_CONN_CLOSED;

	addElementToList(&CONNPOOL, conn);

	CONNPOOL_NEXTID++;		

	unlockMutex(&connpool_mtx);

	if (RUDP_CONN_DEBUG)
		printf("Connection created with id %d.\n", conn->connid);

	return conn;
}

Connection *getConnectionById(const int connid) {
	Connection *conn = NULL;
	ListElement *curr = NULL;

	lockMutex(&connpool_mtx);

	curr = CONNPOOL.head;

	while (curr) {

		if (((Connection *) curr->value)->connid == connid) {

			conn = (Connection *) curr->value;

			break;

		}

		curr = curr->next;
	}	

	unlockMutex(&connpool_mtx);

	return conn;		
}

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {
	
	if (conn->record->state != RUDP_CONN_CLOSED) {

		fprintf(stderr, "Cannot set connection listening: connection not closed.\n");

		exit(EXIT_FAILURE);
	}

	conn->record->sock = openSocket();

	setSocketReusable(conn->record->sock);
	
	bindSocket(conn->record->sock, &laddr);

	conn->record->state = RUDP_CONN_LISTEN;

	if (RUDP_CONN_DEBUG)
		printf("Connection successfully set for listening.\n");
}

ConnectionId acceptSynchonization(Connection *lconn) {
	Connection *aconn = NULL;
	struct sockaddr_in caddr;	
	Segment syn;
	Segment synack; 
	Segment acksynack;
	char *ssyn = NULL;
	char *ssynack = NULL;
	char *sacksynack = NULL;
	int asock;	
	uint32_t seqn;
	uint32_t ackn;	

	if (lconn->record->state != RUDP_CONN_LISTEN) {

		fprintf(stderr, "Cannot accept connection: connection not listening.\n");

		exit(EXIT_FAILURE);
	}	

	while (1) {

		do {

			ssyn = readUnconnectedSocket(lconn->record->sock, &caddr, RUDP_MAX_SGM);			

			syn = deserializeSegment(ssyn);

			if (RUDP_CONN_DEBUG)
				printInSegment(caddr, syn);

			free(ssyn);

		} while (syn.hdr.ctrl != RUDP_SYN);

		lconn->record->state = RUDP_CONN_SYN_RCVD;

		seqn = getISN();

		ackn = (syn.hdr.seqn + 1) % RUDP_MAX_SEQN;		

		asock = openSocket();

		setSocketConnected(asock, caddr);	

		setSocketTimeout(asock, ON_READ, RUDP_TIMEOUT_ACK);

		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, seqn, ackn, NULL);	

		ssynack = serializeSegment(synack);

		seqn = (seqn + 1) % RUDP_MAX_SEQN;

		int synackretrans = 0;

		do {			

			writeConnectedSocket(asock, ssynack);				

			if (RUDP_CONN_DEBUG)
				printOutSegment(getSocketPeer(asock), synack);	

			lconn->record->state = RUDP_CONN_SYN_SENT;		
		
			sacksynack = readConnectedSocket(asock, RUDP_MAX_SGM);

			if (sacksynack) {

				acksynack = deserializeSegment(sacksynack);				

				if (RUDP_CONN_DEBUG)
					printInSegment(getSocketPeer(asock), acksynack);	

				free(sacksynack);			

				if ((acksynack.hdr.ctrl == RUDP_ACK) &
					(acksynack.hdr.seqn == ackn) &
					(acksynack.hdr.ackn == seqn)) {

					free(ssynack);
		
					aconn = createConnection();						

					aconn->record->outbox = createOutbox(seqn, RUDP_MAX_WNDS);

					aconn->record->inbox = createInbox(ackn, RUDP_MAX_WNDS);

					aconn->record->sock = asock;

					setSocketTimeout(asock, ON_READ, 0);	

					aconn->record->timer = createTimer(timeoutHandler, aconn->record);

					setTimer(aconn->record->timer, RUDP_TIMEOUT_ACK, TIMER_PERIODIC);

					aconn->manager = createThread(managerLoop, aconn->record, THREAD_DETACHED);

					aconn->record->state = RUDP_CONN_ESTABLISHED;

					if (RUDP_CONN_DEBUG)
						printf("Connection acceptance succeed.\n");

					return aconn->connid;			
				}
			}

			synackretrans++;

		} while (synackretrans <= RUDP_MAX_RETRANS);

		free(ssynack);				

		closeSocket(asock);

		lconn->record->state = RUDP_CONN_LISTEN;

		if (RUDP_CONN_DEBUG)
			printf("Connection acceptance failed.\n");		
	}
}

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr) {
	struct sockaddr_in aaddr;	
	Segment syn;
	Segment synack;
	Segment acksynack;
	char *ssyn = NULL;
	char *ssynack = NULL;
	char *sacksynack = NULL;
	int asock;
	uint32_t seqn;
	uint32_t ackn;	

	if (conn->record->state != RUDP_CONN_CLOSED) {

		fprintf(stderr, "Cannot synchronize connection: connection not closed.\n");

		exit(EXIT_FAILURE);
	}	

	asock = openSocket();

	setSocketTimeout(asock, ON_READ, 3 * RUDP_TIMEOUT_ACK);

	int connattempts = 1;

	do {	

		if (RUDP_CONN_DEBUG)
			printf("Connection attempt: %d.\n", connattempts);	

		seqn = getISN();

		ackn = 0;				

		syn = createSegment(RUDP_SYN, 0, 0, seqn, ackn, NULL);

		ssyn = serializeSegment(syn);	

		seqn = (seqn + 1) % RUDP_MAX_SEQN;

		int synretrans = 0;

		do {

			writeUnconnectedSocket(asock, laddr, ssyn);		

			if (RUDP_CONN_DEBUG)
				printOutSegment(laddr, syn);

			conn->record->state = RUDP_CONN_SYN_SENT;

			ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_MAX_SGM);

			if (ssynack) {

				synack = deserializeSegment(ssynack);

				free(ssynack);

				if (RUDP_CONN_DEBUG)
					printInSegment(aaddr, synack);

				if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &
					(synack.hdr.ackn == seqn)) {					

					conn->record->state = RUDP_CONN_SYN_RCVD;

					free(ssyn);

					setSocketConnected(asock, aaddr);

					ackn = (synack.hdr.seqn + 1) % RUDP_MAX_SEQN;

					acksynack = createSegment(RUDP_ACK, 0, 0, seqn, ackn, NULL);

					sacksynack = serializeSegment(acksynack);

					seqn = (seqn + 1) % RUDP_MAX_SEQN;

					writeConnectedSocket(asock, sacksynack);

					if (RUDP_CONN_DEBUG)				
						printOutSegment(aaddr, acksynack);

					free(sacksynack);				

					conn->record->outbox = createOutbox(seqn, RUDP_MAX_WNDS);

					conn->record->inbox = createInbox(ackn, RUDP_MAX_WNDS);

					conn->record->sock = asock;

					setSocketTimeout(asock, ON_READ, 0);	

					conn->record->timer = createTimer(timeoutHandler, conn->record);

					setTimer(conn->record->timer, RUDP_TIMEOUT_ACK, TIMER_PERIODIC);

					conn->manager = createThread(managerLoop, conn->record, THREAD_DETACHED);

					conn->record->state = RUDP_CONN_ESTABLISHED;

					if (RUDP_CONN_DEBUG)
						printf("Connection synchronization succeed.\n");

					return conn->connid;					
				}
			}

			synretrans++;

		} while (synretrans <= RUDP_MAX_RETRANS);	

		free(ssyn);

		connattempts++;

	} while (connattempts <= RUDP_MAX_CONN_ATTEMPTS);	

	closeSocket(asock);

	conn->record->state = RUDP_CONN_CLOSED;

	if (RUDP_CONN_DEBUG)
		printf("Connection synchronization failed.\n");

	return -1;
}

void desynchronizeConnection(Connection *conn) {
	//four-way handshake
	destroyConnection(conn);
}
	
void destroyConnection(Connection *conn) {

	if (conn->record->state < RUDP_CONN_ESTABLISHED) {

		conn->record->state = RUDP_CONN_CLOSED;

		puts("Destroying a not established connection");

		destroyMutex(conn->record->conn_mtx);

		destroyConditionVariable(conn->record->conn_cnd);	

		free(conn->record);

	} else {

		conn->record->state = RUDP_CONN_CLOSED;

		freeTimer(conn->record->timer);

		closeSocket(conn->record->sock);

		freeInbox(conn->record->inbox);

		freeOutbox(conn->record->outbox);	

		destroyMutex(conn->record->conn_mtx);

		destroyConditionVariable(conn->record->conn_cnd);

		free(conn->record);
	}	

	lockMutex(&connpool_mtx);	

	ListElement *curr = CONNPOOL.head;

	while (curr) {
		
		if (((Connection *)curr->value)->connid == conn->connid) {

			if (RUDP_CONN_DEBUG)
				printf("Removing connection from connections pool with id %d.\n", conn->connid);
			
			removeElementFromList(&CONNPOOL, curr);

			break;
		}

		curr = curr->next;
	}

	unlockMutex(&connpool_mtx);
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg) {
	Stream *stream = NULL;
	int i;

	stream = createStream(msg);

	lockMutex(conn->record->conn_mtx);

	for (i = 0; i < stream->size; i++)
		submitSegmentToOutbox(conn->record->outbox, stream->segments[i]);

	unlockMutex(conn->record->conn_mtx);
}

char *readInboxMessage(Connection *conn, const size_t size) {
	char *msg = NULL;

	lockMutex(conn->record->conn_mtx);
	
	msg = readInboxBuffer(conn->record->inbox, size);	

	unlockMutex(conn->record->conn_mtx);
	
	return msg;
}

static uint32_t getISN(void) {
	uint32_t isn;
	struct timeval time;

	gettimeofday(&time, NULL);

	isn = (uint32_t) ((1000 * time.tv_sec + time.tv_usec) % RUDP_MAX_SEQN);

	return isn;
}

static void *managerLoop(void *arg) {

	
	// to implement
	return NULL;		
}

static void timeoutHandler(union sigval arg) {

}
