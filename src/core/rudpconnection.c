#include "rudpconnection.h"

static int RUDP_CONN_DEBUG = 0;

/* CONNECTIONS POOL */

static List CONNPOOL = LIST_INITIALIZER;

static int CONNPOOL_NEXTID = 0;

static pthread_mutex_t connpool_mtx = PTHREAD_MUTEX_INITIALIZER;

/* CONNECTION */

static void *managerLoop(void *arg);

static void timeoutHandler(union sigval arg);

Connection *createConnection(void) {
	Connection *conn = NULL;

	lockMutex(&connpool_mtx);

	if (!(conn = malloc(sizeof(Connection))) ||
		!(conn->record = malloc(sizeof(ConnectionRecord))) ||
		!(conn->record->conn_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->record->conn_cnd = malloc(sizeof(pthread_cond_t))))

		ERREXIT("Cannot allocate memory for connection resources.");

	conn->version = RUDP_VERS;

	conn->connid = CONNPOOL_NEXTID;

	initializeMutex(conn->record->conn_mtx);

	initializeConditionVariable(conn->record->conn_cnd);

	setConnectionState(conn, RUDP_CONN_CLOS);

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
	
	if (getConnectionState(conn) != RUDP_CONN_CLOS)
		ERREXIT("Cannot set connection listening: connection not closed.");

	conn->record->sock = openSocket();

	setSocketReusable(conn->record->sock);
	
	bindSocket(conn->record->sock, &laddr);

	setConnectionState(conn, RUDP_CONN_LIST);

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

	if (getConnectionState(lconn) != RUDP_CONN_LIST)
		ERREXIT("Cannot accept connection: connection not listening.");

	while (1) {

		do {

			ssyn = readUnconnectedSocket(lconn->record->sock, &caddr, RUDP_SGMS);			

			if (!ssyn)
				continue;
			
			syn = deserializeSegment(ssyn);

			if (RUDP_CONN_DEBUG)
				printInSegment(caddr, syn);

			free(ssyn);

		} while (syn.hdr.ctrl != RUDP_SYN);

		setConnectionState(lconn, RUDP_CONN_SYNR);

		asock = openSocket();

		setSocketConnected(asock, caddr);	

		setSocketTimeout(asock, ON_READ, RUDP_TIMEO_ACK);

		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, getRandom32(), RUDP_NXTSEQN(syn.hdr.seqn, syn.hdr.plds), NULL);	

		ssynack = serializeSegment(synack);

		int synackretrans = 0;

		do {			

			writeConnectedSocket(asock, ssynack);				

			if (RUDP_CONN_DEBUG)
				printOutSegment(getSocketPeer(asock), synack);

			setConnectionState(lconn, RUDP_CONN_SYNS);		
		
			sacksynack = readConnectedSocket(asock, RUDP_SGMS);

			if (sacksynack) {

				acksynack = deserializeSegment(sacksynack);				

				if (RUDP_CONN_DEBUG)
					printInSegment(getSocketPeer(asock), acksynack);	

				free(sacksynack);			

				if ((acksynack.hdr.ctrl == RUDP_ACK) &
					(acksynack.hdr.seqn == RUDP_NXTSEQN(syn.hdr.seqn, syn.hdr.plds)) &
					(acksynack.hdr.ackn == RUDP_NXTSEQN(synack.hdr.seqn, synack.hdr.plds))) {

					free(ssynack);
		
					aconn = createConnection();						

					aconn->record->outbox = createOutbox(RUDP_NXTSEQN(synack.hdr.seqn, synack.hdr.plds), RUDP_WNDSIZE);

					aconn->record->inbox = createInbox(RUDP_NXTSEQN(acksynack.hdr.seqn, acksynack.hdr.plds), RUDP_WNDSIZE);

					aconn->record->sock = asock;

					setSocketTimeout(asock, ON_READ, 0);	

					aconn->record->timer = createTimer(timeoutHandler, aconn->record);

					setTimer(aconn->record->timer, RUDP_TIMEO_ACK, TIMER_PERIODIC);					

					setConnectionState(aconn, RUDP_CONN_ESTA);

					aconn->manager = createThread(managerLoop, aconn->record, THREAD_DETACHED);

					if (RUDP_CONN_DEBUG)
						printf("Connection acceptance succeed.\n");

					return aconn->connid;			
				}
			}

			synackretrans++;

		} while (synackretrans <= RUDP_RETRANS);

		free(ssynack);				

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CONN_LIST);

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

	if (getConnectionState(conn) != RUDP_CONN_CLOS)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	setSocketTimeout(asock, ON_READ, 3 * RUDP_TIMEO_ACK);

	int connattempts = 1;

	do {	

		if (RUDP_CONN_DEBUG)
			printf("Connection attempt: %d.\n", connattempts);	

		syn = createSegment(RUDP_SYN, 0, 0, getRandom32(), 0, NULL);

		ssyn = serializeSegment(syn);	

		int synretrans = 0;

		do {

			writeUnconnectedSocket(asock, laddr, ssyn);		

			if (RUDP_CONN_DEBUG)
				printOutSegment(laddr, syn);

			setConnectionState(conn, RUDP_CONN_SYNS);

			ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_SGMS);

			if (ssynack) {

				synack = deserializeSegment(ssynack);

				free(ssynack);

				if (RUDP_CONN_DEBUG)
					printInSegment(aaddr, synack);

				if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &
					(synack.hdr.ackn == RUDP_NXTSEQN(syn.hdr.seqn, syn.hdr.plds))) {

					setConnectionState(conn, RUDP_CONN_SYNR);

					free(ssyn);

					setSocketConnected(asock, aaddr);

					acksynack = createSegment(RUDP_ACK, 0, 0, RUDP_NXTSEQN(syn.hdr.seqn, syn.hdr.plds), RUDP_NXTSEQN(synack.hdr.seqn, synack.hdr.plds), NULL);

					sacksynack = serializeSegment(acksynack);

					writeConnectedSocket(asock, sacksynack);

					if (RUDP_CONN_DEBUG)				
						printOutSegment(aaddr, acksynack);

					free(sacksynack);				

					conn->record->outbox = createOutbox(RUDP_NXTSEQN(acksynack.hdr.seqn, acksynack.hdr.plds), RUDP_WNDSIZE);

					conn->record->inbox = createInbox(RUDP_NXTSEQN(synack.hdr.seqn, synack.hdr.plds), RUDP_WNDSIZE);

					conn->record->sock = asock;

					setSocketTimeout(asock, ON_READ, 0);	

					conn->record->timer = createTimer(timeoutHandler, conn->record);

					setTimer(conn->record->timer, RUDP_TIMEO_ACK, TIMER_PERIODIC);

					setConnectionState(conn, RUDP_CONN_ESTA);

					conn->manager = createThread(managerLoop, conn->record, THREAD_DETACHED);					

					if (RUDP_CONN_DEBUG)
						printf("Connection synchronization succeed.\n");

					return conn->connid;					
				}
			}

			synretrans++;

		} while (synretrans <= RUDP_RETRANS);	

		free(ssyn);

		connattempts++;

	} while (connattempts <= RUDP_CONN_ATT);	

	closeSocket(asock);

	setConnectionState(conn, RUDP_CONN_CLOS);

	if (RUDP_CONN_DEBUG)
		printf("Connection synchronization failed.\n");

	return -1;
}

void desynchronizeConnection(Connection *conn) {
	//four-way handshake
	destroyConnection(conn);
}
	
void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CONN_ESTA) {

		setConnectionState(conn, RUDP_CONN_CLOS);

		destroyMutex(conn->record->conn_mtx);

		destroyConditionVariable(conn->record->conn_cnd);	

		free(conn->record);

	} else {

		setConnectionState(conn, RUDP_CONN_CLOS);

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

uint8_t getConnectionState(Connection *conn) {
	uint8_t state;

	lockMutex(conn->record->conn_mtx);

	state = conn->record->state;

	unlockMutex(conn->record->conn_mtx);

	return state;
}

void setConnectionState(Connection *conn, const uint8_t state) {

	lockMutex(conn->record->conn_mtx);

	conn->record->state = state;

	unlockMutex(conn->record->conn_mtx);

	signalConditionVariable(conn->record->conn_cnd);
}

/* MESSAGE COMMUNICATION */

void writeOutboxMessage(Connection *conn, const char *msg, const size_t size) {

	if (getConnectionState(conn) != RUDP_CONN_ESTA)
		ERREXIT("Cannot write message to outbox: connection not established.");

	lockMutex(conn->record->outbox->outbox_mtx);

	while (conn->record->outbox->size != 0) {
		//puts("wait for submission.");
		waitConditionVariable(conn->record->outbox->outbox_cnd, conn->record->outbox->outbox_mtx);
	}		

	writeOutboxUserBuffer(conn->record->outbox, msg, size);

	while (conn->record->outbox->size != 0) {
		//puts("wait for transmission.");
		waitConditionVariable(conn->record->outbox->outbox_cnd, conn->record->outbox->outbox_mtx);
	}

	unlockMutex(conn->record->outbox->outbox_mtx);
}

char *readInboxMessage(Connection *conn, const size_t size) {
	char *msg = NULL;

	if (getConnectionState(conn) != RUDP_CONN_ESTA)
		ERREXIT("Cannot read message from inbox: connection not established.");

	lockMutex(conn->record->inbox->inbox_mtx);

	while (conn->record->inbox->userbuff->csize == 0) {
		//puts("wait for something to read.");
		waitConditionVariable(conn->record->inbox->inbox_cnd, conn->record->inbox->inbox_mtx);
	}
	
	msg = readInboxUserBuffer(conn->record->inbox, size);	

	unlockMutex(conn->record->inbox->inbox_mtx);
	
	return msg;
}

static void *managerLoop(void *arg) {
	ConnectionRecord *connrec = (ConnectionRecord *) arg;
	Segment sgm, acksgm;
	char *ssgm = NULL;
	char *sacksgm = NULL;

	while (1) {

		ssgm = readConnectedSocket(connrec->sock, RUDP_SGMS);

		if (ssgm) {

			sgm = deserializeSegment(ssgm);

			if (RUDP_CONN_DEBUG)
				printInSegment(getSocketPeer(connrec->sock), sgm);

			if (sgm.hdr.plds != 0) {

				acksgm = createSegment(RUDP_ACK, 0, 0, 0, RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds), NULL);

				sacksgm = serializeSegment(acksgm);

				writeConnectedSocket(connrec->sock, sacksgm);

				if (RUDP_CONN_DEBUG)
					printOutSegment(getSocketPeer(connrec->sock), acksgm);

				free(sacksgm);
			}

			lockMutex(connrec->outbox->outbox_mtx);

			if (sgm.hdr.ctrl & RUDP_ACK)
				submitAckToOutbox(connrec->outbox, sgm.hdr.ackn);

			unlockMutex(connrec->outbox->outbox_mtx);			

			lockMutex(connrec->inbox->inbox_mtx);

			submitSegmentToInbox(connrec->inbox, sgm);

			//printf("INBOX %s\n", inboxToString(connrec->inbox));

			unlockMutex(connrec->inbox->inbox_mtx);

			signalConditionVariable(connrec->inbox->inbox_cnd);
		}
	}

	return NULL;		
}

static void timeoutHandler(union sigval arg) {
	ConnectionRecord *connrec = (ConnectionRecord *) arg.sival_ptr;
	Segment *retrans = NULL;
	char *ssgm = NULL;
	uint32_t retransno, i;

	lockMutex(connrec->outbox->outbox_mtx);

	retrans = getRetransmittableSegments(connrec->outbox, &retransno);

	if (retransno != 0) {
		
		for (i = 0; i < retransno; i++) {
			
			ssgm = serializeSegment(retrans[i]);			

			writeConnectedSocket(connrec->sock, ssgm);

			if (RUDP_CONN_DEBUG)
				printOutSegment(getSocketPeer(connrec->sock), retrans[i]);

			free(ssgm);
		}
	}

	free(retrans);

	unlockMutex(connrec->outbox->outbox_mtx);

	signalConditionVariable(connrec->outbox->outbox_cnd);	
}
