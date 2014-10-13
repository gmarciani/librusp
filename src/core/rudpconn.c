#include "rudpconn.h"

static int RUDP_CON_DEBUG = 1;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

static int CONPOOL_NXTID = 0;

static pthread_mutex_t CONPOOL_MTX = PTHREAD_MUTEX_INITIALIZER;

static Connection *allocateConnectionInPool(Connection *conn);

static void deallocateConnectionInPool(Connection *conn);

/* CONNECTION */

static void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double extRTT);

/* CONNECTION TIMEOUT */

static long double getTimeout(Connection *conn);

static void setTimeout(Connection *conn, const long double sampleRTT);

/* SEGMENT I/O */

static void sendSegment(Connection *conn, const Segment sgm);

static int receiveSegment(Connection *conn, Segment *sgm);

/* CONNECTION THREADS */

static void *sndBufferizerLoop(void *arg);

static void *sndSliderLoop(void *arg);

static void timeoutHandler(union sigval arg);

static void *rcvBufferizerLoop(void *arg);

static void *rcvSliderLoop(void *arg);

static void *acceptDesynchronization(void *arg);

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
		!(conn->timeo_mtx = malloc(sizeof(pthread_mutex_t))))
		ERREXIT("Cannot allocate memory for connection resources.");

	initializeMutex(conn->state_mtx);

	initializeConditionVariable(conn->state_cnd);

	initializeMutex(conn->sock_mtx);

	initializeMutex(conn->timeo_mtx);

	setConnectionState(conn, RUDP_CON_CLOS);

	allocateConnectionInPool(conn);

	return conn;
}

static void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double extRTT) {

	conn->sock = sock;

	setSocketTimeout(conn->sock, ON_READ, 0.0);

	setSocketConnected(conn->sock, paddr);	

	conn->extRTT = extRTT;

	conn->devRTT = 0.0;

	conn->timeo = extRTT;

	conn->sndwndb = sndwndb;

	conn->sndwnde = sndwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->sndnext = sndwndb;

	conn->sndbuff = createBuffer();

	conn->sndsgmbuff = createTSegmentBuffer();

	conn->rcvwndb = rcvwndb;

	conn->rcvwnde = rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->rcvbuff = createBuffer();

	conn->rcvsgmbuff = createSegmentBuffer();	

	conn->sndbufferizer = createThread(sndBufferizerLoop, conn, THREAD_DETACHED);

	conn->sndslider = createThread(sndSliderLoop, conn, THREAD_DETACHED);

	conn->rcvbufferizer = createThread(rcvBufferizerLoop, conn, THREAD_DETACHED);

	conn->rcvslider = createThread(rcvSliderLoop, conn, THREAD_DETACHED);
}

static long double getTimeout(Connection *conn) {
	long double timeo;

	lockMutex(conn->timeo_mtx);

	timeo = conn->timeo;

	unlockMutex(conn->timeo_mtx);

	return timeo;
}

static void setTimeout(Connection *conn, const long double sampleRTT) {
	lockMutex(conn->timeo_mtx);

	conn->extRTT = RUDP_EXTRTT(conn->extRTT, sampleRTT);

	conn->devRTT = RUDP_DEVRTT(conn->devRTT, conn->extRTT, sampleRTT);

	conn->timeo = RUDP_TIMEO(conn->extRTT, conn->devRTT);

	unlockMutex(conn->timeo_mtx);

	//printf("TIMEOUT: %LF\n", conn->timeo);
}

int getConnectionState(Connection *conn) {
	int state;

	lockMutex(conn->state_mtx);

	state = conn->state;

	unlockMutex(conn->state_mtx);

	return state;
}

void setConnectionState(Connection *conn, const int state) {

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
	struct timespec start, end;
	long double sampleRTT;	

	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	setSocketTimeout(asock, ON_READ, RUDP_SAMPLRTT);

	int connattempts = 1;

	do {

		syn = createSegment(RUDP_SYN, 0, 0, 0/*getISN(getSocketLocal(asock), laddr)*/, 0, NULL);

		ssyn = serializeSegment(syn);	

		int synretrans = -1;

		do {

			synretrans++;

			clock_gettime(CLOCK_MONOTONIC, &start);

			writeUnconnectedSocket(asock, laddr, ssyn);		

			if (RUDP_CON_DEBUG)
				printOutSegment(laddr, syn);

			setConnectionState(conn, RUDP_CON_SYNS);

			ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_SGMS);

			if (ssynack == NULL) {
				if (RUDP_CON_DEBUG)
					puts("SYNACK DROPPED");
				continue;
			}

			clock_gettime(CLOCK_MONOTONIC, &end);

			sampleRTT = getElapsed(start, end);

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

				setupConnection(conn, asock, aaddr, acksynack.hdr.seqn, acksynack.hdr.ackn, sampleRTT);				

				setConnectionState(conn, RUDP_CON_ESTA);

				return conn->connid;					
			}			

		} while (synretrans <= RUDP_CON_RETR);	

		free(ssyn);

		connattempts++;

	} while (connattempts <= RUDP_CON_ATTS);	

	closeSocket(asock);

	setConnectionState(conn, RUDP_CON_CLOS);

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
	struct timespec start, end;
	long double sampleRTT;

	if (getConnectionState(lconn) != RUDP_CON_LIST)
		ERREXIT("Cannot accept incoming connections: connection not listening.");

	while (1) {

		do {

			ssyn = readUnconnectedSocket(lconn->sock, &caddr, RUDP_SGMS);			

			if (ssyn == NULL) {
				if (RUDP_CON_DEBUG)
					puts("SEGMENT DROPPED");
				continue;
			}				
			
			syn = deserializeSegment(ssyn);

			if (RUDP_CON_DEBUG)
				printInSegment(caddr, syn);

			free(ssyn);

		} while (syn.hdr.ctrl != RUDP_SYN);

		setConnectionState(lconn, RUDP_CON_SYNR);

		asock = openSocket();

		setSocketTimeout(asock, ON_READ, RUDP_SAMPLRTT);
	
		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, 10/*getISN(getSocketLocal(asock), caddr)*/, RUDP_NXTSEQN(syn.hdr.seqn, 1), NULL); 

		ssynack = serializeSegment(synack);

		int synackretrans = -1;

		do {			

			synackretrans++;

			clock_gettime(CLOCK_MONOTONIC, &start);

			writeUnconnectedSocket(asock, caddr, ssynack);			

			if (RUDP_CON_DEBUG)
				printOutSegment(caddr, synack);

			setConnectionState(lconn, RUDP_CON_SYNS);	

			sacksynack = readUnconnectedSocket(asock, &caddr, RUDP_SGMS);

			if (sacksynack == NULL) {
				if (RUDP_CON_DEBUG)
					puts("ACK SYNACK DROPPED");
				continue;
			}

			clock_gettime(CLOCK_MONOTONIC, &end);

			sampleRTT = getElapsed(start, end);

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

				setupConnection(aconn, asock, caddr, acksynack.hdr.ackn, acksynack.hdr.seqn, sampleRTT);

				setConnectionState(aconn, RUDP_CON_ESTA);

				return aconn->connid;			
			}			

		} while (synackretrans <= RUDP_CON_RETR);

		free(ssynack);				

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CON_LIST);	
	}
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

	setConnectionState(conn, RUDP_CON_CLOS);

	destroyMutex(conn->state_mtx);

	destroyConditionVariable(conn->state_cnd);	

	destroyMutex(conn->timeo_mtx);

	destroyMutex(conn->sock_mtx);		

	closeSocket(conn->sock);

	if (getConnectionState(conn) >= RUDP_CON_ESTA) {

		freeTSegmentBuffer(conn->sndsgmbuff);

		freeSegmentBuffer(conn->rcvsgmbuff);

		freeBuffer(conn->sndbuff);		

		freeBuffer(conn->rcvbuff);
	}	

	deallocateConnectionInPool(conn);
}

/* MESSAGE COMMUNICATION */

void writeMessage(Connection *conn, const char *msg, const size_t size) {

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot write message: connection not established.");

	lockMutex(conn->sndbuff->mtx);

	while (conn->sndbuff->size != 0)
		waitConditionVariable(conn->sndbuff->remove_cnd, conn->sndbuff->mtx);

	writeBuffer(conn->sndbuff, msg, size);	

	unlockMutex(conn->sndbuff->mtx);

	signalConditionVariable(conn->sndbuff->insert_cnd);

	lockMutex(conn->sndsgmbuff->mtx);

	do 
		waitConditionVariable(conn->sndsgmbuff->remove_cnd, conn->sndsgmbuff->mtx);
	while (conn->sndsgmbuff->size != 0);

	unlockMutex(conn->sndsgmbuff->mtx);
}

char *readMessage(Connection *conn, const size_t size) {
	char *msg = NULL;

	if (getConnectionState(conn) != RUDP_CON_ESTA)
		ERREXIT("Cannot read message: connection not established.");

	lockMutex(conn->rcvbuff->mtx);

	while (conn->rcvbuff->size < size)
		waitConditionVariable(conn->rcvbuff->insert_cnd, conn->rcvbuff->mtx);
	
	msg = readBuffer(conn->rcvbuff, size);	

	unlockMutex(conn->rcvbuff->mtx);

	signalConditionVariable(conn->rcvbuff->remove_cnd);
	
	return msg;
}

static void sendSegment(Connection *conn, const Segment sgm) {
	char *ssgm = NULL;

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, ssgm);

	unlockMutex(conn->sock_mtx);

	if (RUDP_CON_DEBUG)
		printOutSegment(getSocketPeer(conn->sock), sgm);	

	free(ssgm);
}

static int receiveSegment(Connection *conn, Segment *sgm) {
	char *ssgm = NULL;

	ssgm = readConnectedSocket(conn->sock, RUDP_SGMS);

	if (!ssgm)
		return -1;

	*sgm = deserializeSegment(ssgm);

	if (RUDP_CON_DEBUG)
		printInSegment(getSocketPeer(conn->sock), *sgm);	

	free(ssgm);	

	return 0;
}

/* CONNECTION THREADS */

static void *sndBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	char *pld = NULL;
	long double timeout;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		lockMutex(conn->sndbuff->mtx);

		while (conn->sndbuff->size == 0)
			waitConditionVariable(conn->sndbuff->insert_cnd, conn->sndbuff->mtx);

		lockMutex(conn->sndsgmbuff->mtx);		

		while ((conn->sndwnde - conn->sndnext) < RUDP_PLDS)
			waitConditionVariable(conn->sndsgmbuff->remove_cnd, conn->sndsgmbuff->mtx);

		pld = readBuffer(conn->sndbuff, RUDP_PLDS);

		unlockMutex(conn->sndbuff->mtx);	

		Segment sgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, conn->rcvwndb, pld);

		free(pld);

		conn->sndnext = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);	

		TSegmentBufferElement *timeoutsgm = addTSegmentBuffer(conn->sndsgmbuff, sgm, RUDP_SGM_NACK);
		
		sendSegment(conn, sgm);

		TimeoutObject timeoutobj = {.conn = conn, .elem = timeoutsgm};

		timeout = getTimeout(conn);

		setTSegmentBufferElementTimeout(timeoutsgm, timeout, timeout, timeoutHandler, &timeoutobj, sizeof(TimeoutObject));

		unlockMutex(conn->sndsgmbuff->mtx);	

		signalConditionVariable(conn->sndsgmbuff->insert_cnd);	
	}

	return NULL;
}

static void *sndSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	long double sampleRTT;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		lockMutex(conn->sndsgmbuff->mtx);	

		while (conn->sndsgmbuff->size == 0)
			waitConditionVariable(conn->sndsgmbuff->insert_cnd, conn->sndsgmbuff->mtx);

		while (conn->sndsgmbuff->head->status != RUDP_SGM_YACK)
			waitConditionVariable(conn->sndsgmbuff->status_cnd, conn->sndsgmbuff->mtx);

		conn->sndwndb = RUDP_NXTSEQN(conn->sndwndb, conn->sndsgmbuff->head->segment.hdr.plds);

		conn->sndwnde = RUDP_NXTSEQN(conn->sndwnde, conn->sndsgmbuff->head->segment.hdr.plds);

		sampleRTT = removeTSegmentBuffer(conn->sndsgmbuff, conn->sndsgmbuff->head);	

		unlockMutex(conn->sndsgmbuff->mtx);	

		if (sampleRTT != -1) {

			setTimeout(conn, sampleRTT);

			broadcastConditionVariable(conn->sndsgmbuff->remove_cnd);
		}	
	}	

	return NULL;
}

static void timeoutHandler(union sigval arg) {
	TimeoutObject *timeoutobj = (TimeoutObject *) arg.sival_ptr;
	Connection *conn = NULL;
	Segment sgm;
	uint8_t sgmstatus;

	if (!timeoutobj)
		return;

	conn = timeoutobj->conn;

	if (!conn)
		return;

	lockMutex(conn->sndsgmbuff->mtx);

	sgmstatus = timeoutobj->elem->status;

	sgm = timeoutobj->elem->segment;

	unlockMutex(conn->sndsgmbuff->mtx);

	if (sgmstatus == RUDP_SGM_YACK)
		return;	

	lockMutex(conn->timeo_mtx);

	conn->timeo = RUDP_TIMEOUP * conn->timeo;

	printf("TIMEOUT (%u):  %LF\n", sgm.hdr.seqn, conn->timeo);

	unlockMutex(conn->timeo_mtx);

	sgm.hdr.ackn = conn->rcvwndb;

	sendSegment(conn, sgm);
}

static void *rcvBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm, acksgm;
	int rcvwndmatch;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		if (receiveSegment(conn, &rcvsgm) == -1) {

			if (RUDP_CON_DEBUG)
				puts("SEGMENT DROPPED");

			continue;
		}

		rcvwndmatch = matchSequenceAgainstWindow(conn->rcvwndb, conn->rcvwnde, rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			if (rcvsgm.hdr.plds != 0) {

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

				sendSegment(conn, acksgm);

				lockMutex(conn->rcvsgmbuff->mtx);

				addSegmentBuffer(conn->rcvsgmbuff, rcvsgm);

				unlockMutex(conn->rcvsgmbuff->mtx);

				if (rcvsgm.hdr.seqn == conn->rcvwndb)
					signalConditionVariable(conn->rcvsgmbuff->insert_cnd);				
			}

			if (rcvsgm.hdr.ctrl & RUDP_ACK) {

				lockMutex(conn->sndsgmbuff->mtx);

				TSegmentBufferElement *ackedelem = findTSegmentBufferByAck(conn->sndsgmbuff, rcvsgm.hdr.ackn);

				if (ackedelem) {

					ackedelem->status = RUDP_SGM_YACK;

					setTimer(ackedelem->timer, 0.0, 0.0);	

					signalConditionVariable(conn->sndsgmbuff->status_cnd);					
				}

				unlockMutex(conn->sndsgmbuff->mtx);
			}			

		} else if (rcvwndmatch == -1 && rcvsgm.hdr.plds != 0) {

			acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

			sendSegment(conn, acksgm);

		}	
	}

	return NULL;
}

static void *rcvSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		lockMutex(conn->rcvsgmbuff->mtx);

		while (conn->rcvsgmbuff->size == 0)
			waitConditionVariable(conn->rcvsgmbuff->insert_cnd, conn->rcvsgmbuff->mtx);

		int flag = 1;

		while (flag) {

			flag = 0;

			SegmentBufferElement *curr = conn->rcvsgmbuff->head;

			while (curr) {

				if (curr->segment.hdr.seqn == conn->rcvwndb) {

					flag = 1;

					lockMutex(conn->rcvbuff->mtx);

					writeBuffer(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

					unlockMutex(conn->rcvbuff->mtx);

					signalConditionVariable(conn->rcvbuff->insert_cnd);				

					conn->rcvwndb = RUDP_NXTSEQN(conn->rcvwndb, curr->segment.hdr.plds);

					conn->rcvwnde = RUDP_NXTSEQN(conn->rcvwnde, curr->segment.hdr.plds);	

					removeSegmentBuffer(conn->rcvsgmbuff, curr);			

					break;
				}
	
				curr = curr->next;
			}
		}

		unlockMutex(conn->rcvsgmbuff->mtx);						
	}	

	return NULL;
}
