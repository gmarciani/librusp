#include "conn.h"

/* DEBUG */

static int DEBUG = 1;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

static int CONPOOL_NXTID = 0;

static pthread_mutex_t CONPOOL_MTX = PTHREAD_MUTEX_INITIALIZER;

/* OUTBOX THREADS */

static void *sndBufferizerLoop(void *arg);

static void *sndSliderLoop(void *arg);

static void timeoutHandler(union sigval arg);

/* INBOX THREADS */

static void *rcvBufferizerLoop(void *arg);

static void *rcvSliderLoop(void *arg);

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (!(conn = malloc(sizeof(Connection))))
		ERREXIT("Cannot allocate memory for connection.");

	conn->state_mtx = createMutex();

	conn->state_cnd = createConditionVariable();

	conn->sock_mtx = createMutex();

	conn->timeo_mtx = createMutex();

	setConnectionState(conn, RUDP_CON_CLOS);

	allocateConnectionInPool(conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {
		
		setConnectionState(conn, RUDP_CON_CLOS);

		destroyMutex(conn->state_mtx);

		destroyConditionVariable(conn->state_cnd);	

		destroyMutex(conn->timeo_mtx);

		destroyMutex(conn->sock_mtx);		

		closeSocket(conn->sock);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);

		joinThread(conn->sndbufferizer);

		joinThread(conn->sndslider);

		joinThread(conn->rcvbufferizer);

		joinThread(conn->rcvslider);

		freeTSegmentBuffer(conn->sndsgmbuff);

		freeSegmentBuffer(conn->rcvsgmbuff);

		freeBuffer(conn->sndbuff);		

		freeBuffer(conn->rcvbuff);

		destroyMutex(conn->state_mtx);

		destroyConditionVariable(conn->state_cnd);	

		destroyMutex(conn->timeo_mtx);

		destroyMutex(conn->sock_mtx);		

		closeSocket(conn->sock);
	}		

	deallocateConnectionInPool(conn);
}

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double extRTT) {

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

	conn->sndbufferizer = createThread(sndBufferizerLoop, conn, THREAD_JOINABLE);

	conn->sndslider = createThread(sndSliderLoop, conn, THREAD_JOINABLE);

	conn->rcvbufferizer = createThread(rcvBufferizerLoop, conn, THREAD_JOINABLE);

	conn->rcvslider = createThread(rcvSliderLoop, conn, THREAD_JOINABLE);
}

short getConnectionState(Connection *conn) {
	short state;

	lockMutex(conn->state_mtx);

	state = conn->state;

	unlockMutex(conn->state_mtx);

	return state;
}

void setConnectionState(Connection *conn, const short state) {

	lockMutex(conn->state_mtx);

	conn->state = state;

	unlockMutex(conn->state_mtx);

	signalConditionVariable(conn->state_cnd);
}

long double getTimeout(Connection *conn) {
	long double timeo;

	lockMutex(conn->timeo_mtx);

	timeo = conn->timeo;

	unlockMutex(conn->timeo_mtx);

	return timeo;
}

void setTimeout(Connection *conn, const long double sampleRTT) {
	lockMutex(conn->timeo_mtx);

	conn->extRTT = RUDP_EXTRTT(conn->extRTT, sampleRTT);

	conn->devRTT = RUDP_DEVRTT(conn->devRTT, conn->extRTT, sampleRTT);

	conn->timeo = RUDP_TIMEO(conn->extRTT, conn->devRTT);

	unlockMutex(conn->timeo_mtx);
}

/* SEGMENTS I/O */

void sendSegment(Connection *conn, const Segment sgm) {
	char *ssgm = NULL;

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, ssgm);

	unlockMutex(conn->sock_mtx);

	DBGFUNC(DEBUG, printOutSegment(getSocketPeer(conn->sock), sgm));	

	free(ssgm);
}

int receiveSegment(Connection *conn, Segment *sgm) {
	char *ssgm = NULL;

	ssgm = readConnectedSocket(conn->sock, RUDP_SGMS);

	if (!ssgm)
		return -1;

	*sgm = deserializeSegment(ssgm);

	DBGFUNC(DEBUG, printInSegment(getSocketPeer(conn->sock), *sgm));

	free(ssgm);	

	return 0;
}

/* OUTBOX THREADS */

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

		TimeoutObject timeoutobj = {.conn = conn};

		TSegmentBufferElement *timeoutsgm = addTSegmentBuffer(conn->sndsgmbuff, sgm);
		
		sendSegment(conn, sgm);		

		timeout = getTimeout(conn);

		setTimer(timeoutsgm, timeout, 0);

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

		removeTSegmentBuffer(conn->sndsgmbuff, conn->sndsgmbuff->head);

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

	lockMutex(conn->sndsgmbuff->mtx);		

	sgmstatus = timeoutobj->elem->status;

	sgm = timeoutobj->elem->segment;	

	DBGPRINT(DEBUG, "TIMEOUT(%LF) %u", conn->timeo, sgm.hdr.seqn);

	if (sgmstatus == RUDP_SGM_YACK) {

		unlockMutex(conn->sndsgmbuff->mtx);

		return;
	}

	//conn->timeo = RUDP_TIMEOUP * conn->timeo;	

	sgm.hdr.ackn = conn->rcvwndb;

	unlockMutex(conn->sndsgmbuff->mtx);

	sendSegment(conn, sgm);
}

/* INBOX THREADS */

static void *rcvBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm, acksgm;
	int rcvwndmatch;

	while (getConnectionState(conn) != RUDP_CON_CLOS) {

		if (receiveSegment(conn, &rcvsgm) == -1) {

			DBGPRINT(DEBUG, "SEGMENT DROPPED");

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

/* CONNECTIONS POOL */

Connection *allocateConnectionInPool(Connection *conn) {

	lockMutex(&CONPOOL_MTX);

	conn->connid = CONPOOL_NXTID;

	addElementToList(&CONPOOL, conn);

	CONPOOL_NXTID++;		

	unlockMutex(&CONPOOL_MTX);

	return conn;
}

void deallocateConnectionInPool(Connection *conn) {

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

Connection *getConnectionById(const ConnectionId connid) {
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
