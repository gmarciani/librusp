#include "rudpconnection.h"

static int RUDP_CON_DEBUG = 1;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

static int CONPOOL_NXTID = 0;

static pthread_mutex_t CONPOOL_MTX = PTHREAD_MUTEX_INITIALIZER;

static Connection *allocateConnectionInPool(Connection *conn);

static void deallocateConnectionInPool(Connection *conn);

/* CONNECTION */

static void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb);

static uint32_t getISN(const struct sockaddr_in laddr, const struct sockaddr_in paddr);

static void *acceptDesynchronization(void *arg);

/* CONNECTION THREADS */

static void *senderLoop(void *arg);

static void timeoutHandler(union sigval arg);

static void *receiverLoop(void *arg);

//static void *sendAck(void *arg);

//static void *submitAck(void *arg);

//static void *bufferizeSegment(void *arg);

/* CONNECTIONS POOL */

static Connection *allocateConnectionInPool(Connection *conn) {

	lockMutex(&CONPOOL_MTX);

	conn->connid = CONPOOL_NXTID;

	addElementToList(&CONPOOL, conn);

	CONPOOL_NXTID++;		

	unlockMutex(&CONPOOL_MTX);

	return conn;
}

static void deallocateConnectionInPool(Connection *conn) {

	lockMutex(&CONPOOL_MTX);	

	ListElement *curr = CONPOOL.head;

	while (curr) {
		
		if (((Connection *)curr->value)->connid == conn->connid) {
			
			removeElementFromList(&CONPOOL, curr);

			break;
		}

		curr = curr->next;
	}

	unlockMutex(&CONPOOL_MTX);
}

Connection *getConnectionById(const int connid) {
	Connection *conn = NULL;
	ListElement *curr = NULL;

	lockMutex(&CONPOOL_MTX);

	curr = CONPOOL.head;

	while (curr) {

		if (((Connection *) curr->value)->connid == connid) {

			conn = (Connection *) curr->value;

			break;
		}

		curr = curr->next;
	}	

	unlockMutex(&CONPOOL_MTX);

	return conn;		
}

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (!(conn = malloc(sizeof(Connection))) ||
		!(conn->state_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->state_cnd = malloc(sizeof(pthread_cond_t))) ||
		!(conn->sock_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->sndwnd_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->sndnext_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(conn->rcvwnd_mtx = malloc(sizeof(pthread_mutex_t))))
		ERREXIT("Cannot allocate memory for connection resources.");

	initializeMutex(conn->state_mtx);

	initializeConditionVariable(conn->state_cnd);

	initializeMutex(conn->sock_mtx);

	initializeMutex(conn->sndwnd_mtx);

	initializeMutex(conn->sndnext_mtx);

	initializeMutex(conn->rcvwnd_mtx);

	setConnectionState(conn, RUDP_CON_CLOS);

	allocateConnectionInPool(conn);

	return conn;
}

static void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb) {

	conn->sock = sock;

	setSocketTimeout(conn->sock, ON_READ, 0);

	setSocketConnected(conn->sock, paddr);	

	conn->sndwndb = sndwndb;

	conn->sndwnde = sndwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->sndnext = sndwndb;

	conn->sndtime = RUDP_TIME_ACK;

	conn->sndbuff = createBuffer();

	conn->sndsgmbuff = createTSegmentBuffer();

	conn->rcvwndb = rcvwndb;

	conn->rcvwnde = rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->rcvbuff = createBuffer();

	conn->rcvsgmbuff = createSegmentBuffer();	

	conn->sender = createThread(senderLoop, conn, THREAD_JOINABLE);

	conn->receiver = createThread(receiverLoop, conn, THREAD_JOINABLE);
}

uint8_t getConnectionState(Connection *conn) {
	uint8_t state;

	lockMutex(conn->state_mtx);

	state = conn->state;

	unlockMutex(conn->state_mtx);

	return state;
}

void setConnectionState(Connection *conn, const uint8_t state) {

	lockMutex(conn->state_mtx);

	conn->state = state;

	unlockMutex(conn->state_mtx);

	signalConditionVariable(conn->state_cnd);
}

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {
	
	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot setup listening connection: connection not closed.");

	conn->sock = openSocket();

	setSocketReusable(conn->sock);
	
	bindSocket(conn->sock, &laddr);

	setConnectionState(conn, RUDP_CON_LIST);
}

/* SYNCHRONIZATION */

int synchronizeConnection(Connection *conn, const struct sockaddr_in laddr) {
	struct sockaddr_in aaddr;	
	Segment syn;
	Segment synack;
	Segment acksynack;
	char *ssyn = NULL;
	char *ssynack = NULL;
	char *sacksynack = NULL;
	int asock;	

	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	setSocketTimeout(asock, ON_READ, 3 * RUDP_TIME_ACK);

	int connattempts = 1;

	do {	

		if (RUDP_CON_DEBUG)
			printf("Connection attempt: %d.\n", connattempts);	

		syn = createSegment(RUDP_SYN, 0, 0, 0/*getISN(getSocketLocal(asock), laddr)*/, 0, NULL);

		ssyn = serializeSegment(syn);	

		int synretrans = -1;

		do {

			synretrans++;

			writeUnconnectedSocket(asock, laddr, ssyn);		

			if (RUDP_CON_DEBUG)
				printOutSegment(laddr, syn);

			setConnectionState(conn, RUDP_CON_SYNS);

			ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_SGMS);

			if (ssynack == NULL)
				continue;

			synack = deserializeSegment(ssynack);

			free(ssynack);

			if (RUDP_CON_DEBUG)
				printInSegment(aaddr, synack);

			if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &
				(synack.hdr.ackn == RUDP_NXTSEQN(syn.hdr.seqn, 1))) {

				setConnectionState(conn, RUDP_CON_SYNR);

				free(ssyn);

				acksynack = createSegment(RUDP_ACK, 0, 0, RUDP_NXTSEQN(syn.hdr.seqn, 1), RUDP_NXTSEQN(synack.hdr.seqn, 1), NULL);

				sacksynack = serializeSegment(acksynack);

				writeUnconnectedSocket(asock, aaddr, sacksynack);

				if (RUDP_CON_DEBUG)				
					printOutSegment(aaddr, acksynack);

				free(sacksynack);					

				setupConnection(conn, asock, aaddr, acksynack.hdr.seqn, acksynack.hdr.ackn);				

				setConnectionState(conn, RUDP_CON_ESTA);

				return conn->connid;					
			}			

		} while (synretrans <= RUDP_CON_RETR);	

		free(ssyn);

		connattempts++;

	} while (connattempts <= RUDP_CON_ATTS);	

	closeSocket(asock);

	setConnectionState(conn, RUDP_CON_CLOS);

	if (RUDP_CON_DEBUG)
		printf("Connection synchronization failed.\n");

	return -1;
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

	if (getConnectionState(lconn) != RUDP_CON_LIST)
		ERREXIT("Cannot accept incoming connections: connection not listening.");

	while (1) {

		do {

			ssyn = readUnconnectedSocket(lconn->sock, &caddr, RUDP_SGMS);			

			if (ssyn == NULL)
				continue;
			
			syn = deserializeSegment(ssyn);

			if (RUDP_CON_DEBUG)
				printInSegment(caddr, syn);

			free(ssyn);

		} while (syn.hdr.ctrl != RUDP_SYN);

		setConnectionState(lconn, RUDP_CON_SYNR);

		asock = openSocket();

		setSocketTimeout(asock, ON_READ, RUDP_TIME_ACK);
	
		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, 10/*getISN(getSocketLocal(asock), caddr)*/, RUDP_NXTSEQN(syn.hdr.seqn, 1), NULL); 

		ssynack = serializeSegment(synack);

		int synackretrans = -1;

		do {			

			synackretrans++;

			writeUnconnectedSocket(asock, caddr, ssynack);			

			if (RUDP_CON_DEBUG)
				printOutSegment(caddr, synack);

			setConnectionState(lconn, RUDP_CON_SYNS);	

			sacksynack = readUnconnectedSocket(asock, &caddr, RUDP_SGMS);

			if (sacksynack == NULL) 
				continue;

			acksynack = deserializeSegment(sacksynack);				

			if (RUDP_CON_DEBUG)
				printInSegment(caddr, acksynack);	

			free(sacksynack);			

			if ((acksynack.hdr.ctrl == RUDP_ACK) &
				(acksynack.hdr.seqn == synack.hdr.ackn) &
				(acksynack.hdr.ackn == RUDP_NXTSEQN(synack.hdr.seqn, 1))) {

				free(ssynack);
	
				aconn = createConnection();	

				setConnectionState(aconn, RUDP_CON_SYNS);

				setupConnection(aconn, asock, caddr, acksynack.hdr.ackn, acksynack.hdr.seqn);

				setConnectionState(aconn, RUDP_CON_ESTA);

				return aconn->connid;			
			}			

		} while (synackretrans <= RUDP_CON_RETR);

		free(ssynack);				

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CON_LIST);	
	}
}

static uint32_t getISN(const struct sockaddr_in laddr, const struct sockaddr_in paddr) {
	char *strladdr = NULL;
	char *strpaddr = NULL;
	uint32_t isn;

	strladdr = addressToString(laddr);

	strpaddr = addressToString(paddr);

	isn = (((getMD5(strladdr) + getMD5(strpaddr)) % clock()) + getRandom32()) % RUDP_MAXSEQN;

	free(strladdr);

	free(strpaddr);
	
	return isn;	
}

/* DESYNCHRONIZATION */

void desynchronizeConnection(Connection *conn) {
	destroyConnection(conn);
}

static void *acceptDesynchronization(void *arg) {
	Connection *conn = (Connection *) arg;

	destroyConnection(conn);

	return NULL;
}
	
void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {

		setConnectionState(conn, RUDP_CON_CLOS);

		destroyMutex(conn->state_mtx);

		destroyConditionVariable(conn->state_cnd);	

		destroyMutex(conn->sock_mtx);

		destroyMutex(conn->sndwnd_mtx);

		destroyMutex(conn->sndnext_mtx);

		destroyMutex(conn->rcvwnd_mtx);

		closeSocket(conn->sock);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);	

		freeBuffer(conn->sndbuff);

		freeTSegmentBuffer(conn->sndsgmbuff);

		freeSegmentBuffer(conn->rcvsgmbuff);

		freeBuffer(conn->rcvbuff);

		destroyMutex(conn->state_mtx);

		destroyConditionVariable(conn->state_cnd);

		destroyMutex(conn->sock_mtx);

		destroyMutex(conn->sndwnd_mtx);

		destroyMutex(conn->sndnext_mtx);

		destroyMutex(conn->rcvwnd_mtx);

		closeSocket(conn->sock);
	}	

	deallocateConnectionInPool(conn);
}

/* MESSAGE COMMUNICATION */

void writeMessage(Connection *conn, const char *msg, const size_t size) {

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot write message: connection not established.");

	lockMutex(conn->sndbuff->mtx);

	while (conn->sndbuff->size != 0)
		waitConditionVariable(conn->sndbuff->cnd, conn->sndbuff->mtx);

	writeBuffer(conn->sndbuff, msg, size);	

	unlockMutex(conn->sndbuff->mtx);

	signalConditionVariable(conn->sndbuff->cnd);

	lockMutex(conn->sndbuff->mtx);

	while (conn->sndbuff->size != 0)
		waitConditionVariable(conn->sndbuff->cnd, conn->sndbuff->mtx);

	unlockMutex(conn->sndbuff->mtx);

	puts("EVERYTHING DELIVERED");
}

char *readMessage(Connection *conn, const size_t size) {
	char *msg = NULL;

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot read message: connection not established.");

	lockMutex(conn->rcvbuff->mtx);

	while (conn->rcvbuff->size < size)
		waitConditionVariable(conn->rcvbuff->cnd, conn->rcvbuff->mtx);
	
	msg = readBuffer(conn->rcvbuff, size);	

	unlockMutex(conn->rcvbuff->mtx);
	
	return msg;
}

/* CONNECTION THREADS */

static void *senderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	TSegmentBufferElement *curr = NULL;
	char *tochunk, **chunks, *ssgm = NULL;
	int numchunks, i;
	size_t toread;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		lockMutex(conn->sndbuff->mtx);

		while (conn->sndbuff->size == 0)
			waitConditionVariable(conn->sndbuff->cnd, conn->sndbuff->mtx);

		toread = (conn->sndbuff->size < (conn->sndwnde - conn->sndnext)) ? conn->sndbuff->size : (conn->sndwnde - conn->sndnext);

		tochunk = lookBuffer(conn->sndbuff, toread);

		unlockMutex(conn->sndbuff->mtx);	

		chunks = splitStringBySize(tochunk, RUDP_PLDS, &numchunks);

		lockMutex(conn->sndsgmbuff->mtx);		

		lockMutex(conn->sndnext_mtx);

		for (i = 0; i < numchunks; i++) {			
			
			Segment sgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, conn->rcvwndb, chunks[i]);

			ssgm = serializeSegment(sgm);

			lockMutex(conn->sock_mtx);

			writeConnectedSocket(conn->sock, ssgm);

			unlockMutex(conn->sock_mtx);

			free(ssgm);

			if (RUDP_CON_DEBUG)
				printOutSegment(getSocketPeer(conn->sock), sgm);	

			conn->sndnext = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);			

			SegmentObject sgmobj = {.conn = conn, .sgm = sgm};

			addTSegmentBuffer(conn->sndsgmbuff, sgm, RUDP_SGM_NACK, RUDP_TIME_ACK, RUDP_TIME_ACK, timeoutHandler, &sgmobj, sizeof(SegmentObject));
		}

		unlockMutex(conn->sndnext_mtx);

		while (conn->sndsgmbuff->head->status != RUDP_SGM_YACK)
			waitConditionVariable(conn->sndsgmbuff->cnd, conn->sndsgmbuff->mtx);

		puts("SNDWNDB ACKED: SLIDING SNDWND");

		lockMutex(conn->sndbuff->mtx);

		curr = conn->sndsgmbuff->head;

		while (curr) {

			if (curr->status != RUDP_SGM_YACK)
				break;						

			conn->sndwndb = RUDP_NXTSEQN(conn->sndwndb, curr->segment.hdr.plds);

			conn->sndwnde = RUDP_NXTSEQN(conn->sndwnde, curr->segment.hdr.plds);

			readBuffer(conn->sndbuff, curr->segment.hdr.plds);

			removeTSegmentBuffer(conn->sndsgmbuff, curr);					

			curr = curr->next;
		}		

		unlockMutex(conn->sndbuff->mtx);	

		signalConditionVariable(conn->sndbuff->cnd);	

		unlockMutex(conn->sndsgmbuff->mtx);		

		broadcastConditionVariable(conn->sndsgmbuff->cnd);		
	}

	return NULL;
}

static void timeoutHandler(union sigval arg) {
	SegmentObject *sgmobj = (SegmentObject *) arg.sival_ptr;
	Connection *conn = sgmobj->conn;
	Segment sgm = sgmobj->sgm;
	char *ssgm = NULL;

	if (getConnectionState(conn) == RUDP_CON_CLOS)
		return;

	sgm.hdr.ackn = conn->rcvwndb;

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, ssgm);

	unlockMutex(conn->sock_mtx);

	free(ssgm);

	if (RUDP_CON_DEBUG)
		printOutSegment(getSocketPeer(conn->sock), sgm);	
}

static void *receiverLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm, acksgm;
	char *srcvsgm, *sacksgm = NULL;
	int rcvwndmatch;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		srcvsgm = readConnectedSocket(conn->sock, RUDP_SGMS);

		if (!srcvsgm) {
			puts("SEGMENT DROPPED");
			continue;
		}			

		rcvsgm = deserializeSegment(srcvsgm);

		free(srcvsgm);

		if (RUDP_CON_DEBUG)
			printInSegment(getSocketPeer(conn->sock), rcvsgm);

		rcvwndmatch = matchSequenceAgainstWindow(conn->rcvwndb, conn->rcvwnde, rcvsgm.hdr.seqn);

		if (rcvwndmatch == -1 && rcvsgm.hdr.plds != 0) {

			if (RUDP_CON_DEBUG)
				printf("SEGMENT BEFORE WND: %u\n", rcvsgm.hdr.seqn);

			lockMutex(conn->sndnext_mtx);

			acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

			unlockMutex(conn->sndnext_mtx);

			sacksgm = serializeSegment(acksgm);

			lockMutex(conn->sock_mtx);

			writeConnectedSocket(conn->sock, sacksgm);

			unlockMutex(conn->sock_mtx);

			free(sacksgm);

			if (RUDP_CON_DEBUG)
				printOutSegment(getSocketPeer(conn->sock), acksgm);	

		} else if (rcvwndmatch == 0) {

			if (RUDP_CON_DEBUG)
				printf("SEGMENT INSIDE WND: %u\n", rcvsgm.hdr.seqn);

			if (rcvsgm.hdr.plds != 0) {

				lockMutex(conn->sndnext_mtx);

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

				unlockMutex(conn->sndnext_mtx);

				sacksgm = serializeSegment(acksgm);

				lockMutex(conn->sock_mtx);

				writeConnectedSocket(conn->sock, sacksgm);

				unlockMutex(conn->sock_mtx);

				free(sacksgm);

				if (RUDP_CON_DEBUG)
					printOutSegment(getSocketPeer(conn->sock), acksgm);	

				lockMutex(conn->rcvsgmbuff->mtx);

				addSegmentBuffer(conn->rcvsgmbuff, rcvsgm);

				if (rcvsgm.hdr.seqn == conn->rcvwndb) {

					lockMutex(conn->rcvbuff->mtx);

					int flag = 1;

					while (flag) {
			
						flag = 0;

						SegmentBufferElement *curr = conn->rcvsgmbuff->head;

						while (curr) {

							if (curr->segment.hdr.seqn == conn->rcvwndb) {
	
								flag = 1;
					
								conn->rcvwndb = RUDP_NXTSEQN(conn->rcvwndb, curr->segment.hdr.plds);

								conn->rcvwnde = RUDP_NXTSEQN(conn->rcvwnde, curr->segment.hdr.plds);

								writeBuffer(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

								removeSegmentBuffer(conn->rcvsgmbuff, curr);

								break;
							}
				
							curr = curr->next;
						}
					}

					unlockMutex(conn->rcvbuff->mtx);			

					signalConditionVariable(conn->rcvbuff->cnd);
				}	

				unlockMutex(conn->rcvsgmbuff->mtx);

				signalConditionVariable(conn->rcvsgmbuff->cnd);
			}

			if (rcvsgm.hdr.ctrl & RUDP_ACK) {

				lockMutex(conn->sndsgmbuff->mtx);

				TSegmentBufferElement *ackedelem = findTSegmentBufferByAck(conn->sndsgmbuff, rcvsgm.hdr.ackn);

				if (ackedelem) {

					ackedelem->status = RUDP_SGM_YACK;

					setTimer(ackedelem->timer, 0, 0);	
				}

				unlockMutex(conn->sndsgmbuff->mtx);

				broadcastConditionVariable(conn->sndsgmbuff->cnd);
			}			

		} else {

			if (RUDP_CON_DEBUG)
				printf("SEGMENT DISCARDED: %u\n", rcvsgm.hdr.seqn);

			continue;
		}	
	}

	return NULL;
}
/*
static void *sendAck(void *arg) {
	AckObject *ackobj = (AckObject *) arg;
	Connection *conn = ackobj->conn;
	uint32_t ackn = ackobj->ackn;
	Segment acksgm;
	char *sacksgm = NULL;

	//lockMutex(conn->sndnext_mtx);

	acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, ackn, NULL);

	//unlockMutex(conn->sndnext_mtx);

	sacksgm = serializeSegment(acksgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, sacksgm);

	unlockMutex(conn->sock_mtx);

	free(sacksgm);

	if (RUDP_CON_DEBUG)
		printOutSegment(getSocketPeer(conn->sock), acksgm);	

	return NULL;
}

static void *submitAck(void *arg) {
	AckObject *ackobj = (AckObject *) arg;
	Connection *conn = ackobj->conn;
	uint32_t ackn = ackobj->ackn;
	//int sndwndbmatch = 0;

	lockMutex(conn->sndsgmbuff->mtx);

	TSegmentBufferElement *ackedelem = findTSegmentBufferByAck(conn->sndsgmbuff, ackn);

	if (ackedelem) {

		ackedelem->status = RUDP_SGM_YACK;

		setTimer(ackedelem->timer, 0, 0);	
	}

	unlockMutex(conn->sndsgmbuff->mtx);

	signalConditionVariable(conn->sndsgmbuff->cnd);

	return NULL;
}

static void *bufferizeSegment(void *arg) {
	SegmentObject *sgmobj = (SegmentObject *) arg;
	Connection *conn = sgmobj->conn;
	Segment sgm = sgmobj->sgm;
	int rcvwndbmatch = 0;

	lockMutex(conn->rcvsgmbuff->mtx);

	addSegmentBuffer(conn->rcvsgmbuff, sgm);

	//unlockMutex(conn->rcvsgmbuff->mtx);

	//lockMutex(conn->rcvwnd_mtx);

	rcvwndbmatch = (sgm.hdr.seqn == conn->rcvwndb);

	//unlockMutex(conn->rcvwnd_mtx);

	if (rcvwndbmatch) {

		int flag = 1;

		while (flag) {
			
			flag = 0;

			SegmentBufferElement *curr = conn->rcvsgmbuff->head;

			while (curr) {

				//lockMutex(conn->rcvwnd_mtx);

				rcvwndbmatch = curr->segment.hdr.seqn == conn->rcvwndb;

				//unlockMutex(conn->rcvwnd_mtx);

				if (rcvwndbmatch) {
	
					flag = 1;

					//lockMutex(conn->rcvwnd_mtx);
					
					conn->rcvwndb = RUDP_NXTSEQN(conn->rcvwndb, curr->segment.hdr.plds);

					conn->rcvwnde = RUDP_NXTSEQN(conn->rcvwnde, curr->segment.hdr.plds);

					//unlockMutex(conn->rcvwnd_mtx);

					//lockMutex(conn->rcvbuff->mtx);

					writeBuffer(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

					//unlockMutex(conn->rcvbuff->mtx);

					//lockMutex(conn->rcvsgmbuff->mtx);

					removeSegmentBuffer(conn->rcvsgmbuff, curr);

					//unlockMutex(conn->rcvsgmbuff->mtx);

					break;
				}
				
				curr = curr->next;
			}
		}			

		signalConditionVariable(conn->rcvbuff->cnd);
	}	

	signalConditionVariable(conn->rcvsgmbuff->cnd);

	return NULL;
}*/
