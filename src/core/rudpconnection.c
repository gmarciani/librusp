#include "rudpconnection.h"

static int RUDP_CONN_DEBUG = 1;

/* CONNECTIONS POOL */

static Connection **CONNPOOL = NULL;

static int CONNPOOL_SIZE = 0;

static int CONNPOOL_ACTIVE = 0;

static int CONNPOOL_NEXTID = 0;

static void connectionsPoolDoublingHalving();

static int getConnectionsPoolFirstAvailablePosition();

/* CONNECTION */

//static void sendSegment(Connection *conn, Segment sgm);

//static Segment receiveSegment(Connection *conn);

static pthread_t createManager(ConnectionRecord *record);

static void *managerLoop(void *arg);

static uint32_t getISN();

Connection *createConnection(void) {
	Connection *conn = NULL;
	int poolpos;

	connectionsPoolDoublingHalving();

	poolpos = getConnectionsPoolFirstAvailablePosition();	

	if (poolpos == -1) {
		fprintf(stderr, "Cannot retrieve available position in connections pool.\n");
		exit(EXIT_FAILURE);
	}

	if (!(conn = malloc(sizeof(Connection)))) {
		fprintf(stderr, "Cannot allocate new connection.\n");
		exit(EXIT_FAILURE);
	}

	if (!(conn->record = malloc(sizeof(ConnectionRecord)))) {
		fprintf(stderr, "Cannot allocate connection record for new connection.\n");
		exit(EXIT_FAILURE);
	}

	conn->version = RUDP_VERSION;

	conn->connid = CONNPOOL_NEXTID;

	if (!(conn->record->recordmtx = malloc(sizeof(pthread_mutex_t)))) {
		fprintf(stderr, "Cannot allocate memory for connection mutex.\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(conn->record->recordmtx, NULL) != 0) {
		fprintf(stderr, "Cannot create connection record mutex\n");
		exit(EXIT_FAILURE);
	} 

	conn->record->state = RUDP_CONN_CLOSED;

	CONNPOOL[poolpos] = conn;

	CONNPOOL_NEXTID++;	

	CONNPOOL_SIZE++;

	if (RUDP_CONN_DEBUG)
		printf("Connection created with id: %d\n", conn->connid);

	return conn;
}

Connection *getConnectionById(const int connid) {
	int poolpos;

	for (poolpos = 0; poolpos < CONNPOOL_SIZE; poolpos++) {

		if (CONNPOOL[poolpos]) {

			if (CONNPOOL[poolpos]->connid == connid) {

				if (RUDP_CONN_DEBUG)
					printf("Connection retrieved with id: %d\n", connid);	

				return CONNPOOL[poolpos];
			}				
		}		
	}	

	if (RUDP_CONN_DEBUG)
		printf("Cannot retrieve connection with id: %d\n", connid);	

	return NULL;
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
		printf("Connection successfully set for listening on address %s\n", addressToString(laddr));
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
	uint32_t pisn;
	uint32_t lisn;	

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

		pisn = syn.hdr.seqn;

		lisn = getISN();

		asock = openSocket();

		setSocketConnected(asock, caddr);	

		setSocketTimeout(asock, ON_READ, RUDP_SYNACK_TIMEOUT);

		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, lisn, ((pisn + 1) % RUDP_MAX_SEQN), NULL);	

		ssynack = serializeSegment(synack);

		int retrans = 0;

		do {			

			writeConnectedSocket(asock, ssynack);		

			if (RUDP_CONN_DEBUG)
				printOutSegment(getSocketPeer(asock), synack);

			lconn->record->state = RUDP_CONN_SYN_SENT;
		
			sacksynack = readConnectedSocket(asock, RUDP_MAX_SGM);

			if (sacksynack) {

				acksynack = deserializeSegment(sacksynack);

				free(sacksynack);

				if (RUDP_CONN_DEBUG)
					printInSegment(getSocketPeer(asock), acksynack);				

				if ((acksynack.hdr.ctrl == RUDP_ACK) &&
					(acksynack.hdr.seqn == ((pisn + 1) % RUDP_MAX_SEQN)) &&
					(acksynack.hdr.ackn == ((lisn + 1) % RUDP_MAX_SEQN))) {
		
					aconn = createConnection();	

					if (aconn) {

						free(ssynack);

						aconn->record->outbox = createOutbox(((lisn + 1) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

						aconn->record->inbox = createInbox(((pisn + 2) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

						aconn->record->sock = asock;

						aconn->manager = createManager(aconn->record);

						if (RUDP_CONN_DEBUG)
							printf("Connection acceptance succeed.\n");

						aconn->record->state = RUDP_CONN_ESTABLISHED;

						return aconn->connid;
					}				
				}
			}

			retrans++;

		} while (retrans <= RUDP_MAX_RETRANS);				

		if (RUDP_CONN_DEBUG)
			printf("Connection acceptance failed.\n");

		closeSocket(asock);

		lconn->record->state = RUDP_CONN_LISTEN;		
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
	uint32_t lisn;
	uint32_t pisn;	

	if (conn->record->state != RUDP_CONN_CLOSED) {
		fprintf(stderr, "Cannot synchronize connection: connection not closed.\n");
		exit(EXIT_FAILURE);
	}	

	lisn = getISN();

	asock = openSocket();

	setSocketTimeout(asock, ON_READ, RUDP_SYN_TIMEOUT);		

	syn = createSegment(RUDP_SYN, 0, 0, lisn, 0, NULL);

	ssyn = serializeSegment(syn);

	int retrans = 0;

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

			if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &&
				(synack.hdr.ackn == ((lisn + 1) % RUDP_MAX_SEQN))) {

				conn->record->state = RUDP_CONN_SYN_RCVD;

				setSocketConnected(asock, aaddr);

				pisn = synack.hdr.seqn;

				acksynack = createSegment(RUDP_ACK, 0, 0, ((lisn + 1) % RUDP_MAX_SEQN), ((pisn + 1) % RUDP_MAX_SEQN), NULL);

				sacksynack = serializeSegment(acksynack);

				writeConnectedSocket(asock, sacksynack);

				free(sacksynack);

				if (RUDP_CONN_DEBUG)				
					printOutSegment(aaddr, acksynack);				

				conn->record->outbox = createOutbox(((lisn + 2) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

				conn->record->inbox = createInbox(((pisn + 1) % RUDP_MAX_SEQN), RUDP_MAX_WNDS);

				conn->record->sock = asock;

				setSocketTimeout(asock, ON_READ, RUDP_ACK_TIMEOUT);	

				conn->manager = createManager(conn->record);

				conn->record->state = RUDP_CONN_ESTABLISHED;

				if (RUDP_CONN_DEBUG)
					printf("Connection synchronization succeed.\n");

				return conn->connid;					
			}
		}	

		retrans++;

	} while (retrans <= RUDP_MAX_RETRANS);	

	if (RUDP_CONN_DEBUG)
		printf("Connection synchronization failed.\n");

	closeSocket(asock);

	conn->record->state = RUDP_CONN_CLOSED;

	return -1;
}

void desynchronizeConnection(Connection *conn) {
	//four-way handshake
	destroyConnection(conn);
}
	
void destroyConnection(Connection *conn) {
	int managerRetval;

	conn->record->state = RUDP_CONN_CLOSED;

	if (pthread_join(conn->manager, (void *)&managerRetval) != 0) {
		fprintf(stderr, "Cannot join connection manager.\n");
		exit(EXIT_FAILURE);
	}

	if (managerRetval != 0 && RUDP_CONN_DEBUG)
		fprintf(stderr, "Connection manager reported some errors.\n");

	freeInbox(conn->record->inbox);

	freeOutbox(conn->record->outbox);	

	if (close(conn->record->sock) == -1) {
		fprintf(stderr, "Cannot close socket.\n");
		exit(EXIT_FAILURE);
	}

	free(conn);

	CONNPOOL_SIZE--;	
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg) {
	Stream *stream = NULL;
	int i;

	stream = createStream(msg);

	if (pthread_mutex_lock(conn->record->recordmtx) != 0) {
		fprintf(stderr, "Cannot lock connection record mutex to write to outbox.\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < stream->size; i++)
		submitSegmentToOutbox(conn->record->outbox, stream->segments[i]);

	if (pthread_mutex_unlock(conn->record->recordmtx) != 0) {
		fprintf(stderr, "Cannot unlock connection record mutex to write to outbox.\n");
		exit(EXIT_FAILURE);
	}
}

char *readInboxMessage(Connection *conn, const size_t size) {
	char *msg = NULL;

	if (pthread_mutex_lock(conn->record->recordmtx) != 0) {
		fprintf(stderr, "Cannot lock connection record mutex to read from inbox outbox.\n");
		exit(EXIT_FAILURE);
	}
	
	msg = readInboxBuffer(conn->record->inbox, size);	

	if (pthread_mutex_unlock(conn->record->recordmtx) != 0) {
		fprintf(stderr, "Cannot unlock connection record mutex to read from inbox outbox.\n");
		exit(EXIT_FAILURE);
	}
	
	return msg;
}

/* SEGMENT COMMUNICATION */
/*
static void sendSegment(Connection *conn, Segment sgm) {

	submitSegmentToOutbox(conn->record->outbox, sgm);
}

static Segment receiveSegment(Connection *conn) {	
	Segment sgm;

	sgm = readInboxSegment(conn->record.inbox);

	return sgm;
}*/

static void connectionsPoolDoublingHalving() {
	if (CONNPOOL_SIZE == 0) {

		CONNPOOL_SIZE = 2;

		if (!(CONNPOOL = malloc(sizeof(Connection *) * CONNPOOL_SIZE))) {
			fprintf(stderr, "Cannot allocate connections pool.\n");
			exit(EXIT_FAILURE);
		}		

		if (RUDP_CONN_DEBUG)
			printf("Connections pool successfully allocated with size %d.\n", CONNPOOL_SIZE);
		
		return;

	} else if (CONNPOOL_ACTIVE == CONNPOOL_SIZE - 1) {

		CONNPOOL_SIZE *= 2;
			
		if (!(CONNPOOL = realloc(CONNPOOL, sizeof(Connection *) * CONNPOOL_SIZE))) {
			fprintf(stderr, "Cannot reallocate connections pool.\n");
			exit(EXIT_FAILURE);
		}		

		if (RUDP_CONN_DEBUG)
			printf("Connections pool successfully reallocated with size %d.\n", CONNPOOL_SIZE);

		return;

	} else if (CONNPOOL_ACTIVE <= ceil(CONNPOOL_SIZE * (1/4)) - 1) {

		CONNPOOL_SIZE /= 2;
				
		if (!(CONNPOOL = realloc(CONNPOOL, sizeof(Connection *) * CONNPOOL_SIZE))) {
			fprintf(stderr, "Cannot reallocate connections pool.\n");
			exit(EXIT_FAILURE);
		}		

		if (RUDP_CONN_DEBUG)
			printf("Connections pool successfully reallocated with size %d.\n", CONNPOOL_SIZE);

		return;
	}
}

static int getConnectionsPoolFirstAvailablePosition() {
	int poolpos;

	for (poolpos = 0; poolpos < CONNPOOL_SIZE; poolpos++) {
		if (!CONNPOOL[poolpos])
			return poolpos;
	}

	return -1;
}

static uint32_t getISN() {
	uint32_t isn;
	struct timeval time;

	gettimeofday(&time, NULL);

	isn = (uint32_t) ((1000 * time.tv_sec + time.tv_usec) % RUDP_MAX_SEQN);

	return isn;
}

static pthread_t createManager(ConnectionRecord *record) {
	pthread_t tid;

	if (pthread_create(&tid, NULL, managerLoop, record) != 0) {
		fprintf(stderr, "Cannot create connection manager.\n");
		exit(EXIT_FAILURE);
	} 

	return tid;
}

static void *managerLoop(void *arg) {
	ConnectionRecord *record = (ConnectionRecord *) arg;
	int retval = 0;
	Segment lsgm;
	Segment psgm;
	char *lssgm = NULL;
	char *pssgm = NULL;

	while (1) {

		switch (record->state) {

			case RUDP_CONN_ESTABLISHED:

				pssgm = readConnectedSocket(record->sock, RUDP_MAX_SGM);				

				break;

			case RUDP_CONN_CLOSED:
				pthread_exit((void *)&retval);
				break;
		}
			
	}		
}
