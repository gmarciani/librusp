#include "conn.h"

/* DEBUG */

static int DEBUG = 1;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

static int CONPOOL_NXTID = 0;

static pthread_mutex_t CONPOOL_MTX = PTHREAD_MUTEX_INITIALIZER;

/* OUTBOX */

static void *sndBufferizerLoop(void *arg);

static void timeoutHandler(union sigval arg);

static void sendAck(Connection *conn, const Segment sgm);

/* INBOX */

static void *rcvBufferizerLoop(void *arg);

static void *rcvSliderLoop(void *arg);

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (!(conn = malloc(sizeof(Connection))))
		ERREXIT("Cannot allocate memory for connection.");

	conn->state_rwlock = createRWLock();

	conn->state_cnd = createConditionVariable();

	conn->sock_mtx = createMutex();

	setConnectionState(conn, RUDP_CON_CLOS);

	allocateConnectionInPool(conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {
		
		setConnectionState(conn, RUDP_CON_CLOS);

		freeRWLock(conn->state_rwlock);

		destroyConditionVariable(conn->state_cnd);	

		destroyMutex(conn->sock_mtx);		

		closeSocket(conn->sock);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);

		cancelThread(conn->sndbufferizer);

		cancelThread(conn->rcvbufferizer);

		cancelThread(conn->rcvslider);

		freeTimeout(conn->timeout);

		freeSgmBuff(conn->sndsgmbuff);

		freeSgmBuff(conn->rcvsgmbuff);

		freeStrBuff(conn->sndbuff);		

		freeStrBuff(conn->rcvbuff);

		freeWindow(conn->sndwnd);

		freeWindow(conn->rcvwnd);

		freeRWLock(conn->state_rwlock);

		destroyConditionVariable(conn->state_cnd);

		destroyMutex(conn->sock_mtx);		

		closeSocket(conn->sock);
	}		

	deallocateConnectionInPool(conn);
}

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double sampleRTT) {

	conn->sock = sock;

	setSocketTimeout(conn->sock, ON_READ, 0.0);

	setSocketConnected(conn->sock, paddr);

	conn->timeout = createTimeout(sampleRTT, timeoutHandler, conn);

	conn->sndwnd = createWindow(sndwndb, sndwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	conn->rcvwnd = createWindow(rcvwndb, rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	conn->sndbuff = createStrBuff();

	conn->sndsgmbuff = createSgmBuff();

	conn->rcvbuff = createStrBuff();

	conn->rcvsgmbuff = createSgmBuff();

	conn->sndbufferizer = createThread(sndBufferizerLoop, conn, THREAD_JOINABLE);

	conn->rcvbufferizer = createThread(rcvBufferizerLoop, conn, THREAD_JOINABLE);

	conn->rcvslider = createThread(rcvSliderLoop, conn, THREAD_JOINABLE);
}

short getConnectionState(Connection *conn) {
	short state;

	lockRead(conn->state_rwlock);

	state = conn->state;

	unlockRWLock(conn->state_rwlock);

	return state;
}

void setConnectionState(Connection *conn, const short state) {

	lockWrite(conn->state_rwlock);

	conn->state = state;

	unlockRWLock(conn->state_rwlock);

	broadcastConditionVariable(conn->state_cnd);
}

/* SEGMENTS I/O */

void sendSegment(Connection *conn, Segment sgm) {
	char *ssgm = NULL;

	if (!(sgm.hdr.ctrl & RUDP_ACK)) {

		sgm.hdr.ctrl |= RUDP_ACK;

		sgm.hdr.ackn = getWindowBase(conn->rcvwnd);
	}

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, ssgm);

	DBGFUNC(DEBUG, printOutSegment(getSocketPeer(conn->sock), sgm));

	unlockMutex(conn->sock_mtx);

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

/* OUTBOX */

static void *sndBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	char *pld = NULL;

	while (1) {

		pld = waitStrBuffContent(conn->sndbuff, RUDP_PLDS);

		Segment sgm = createSegment(0, 0, 0, getWindowNext(conn->sndwnd), 0, pld);

		waitWindowSpace(conn->sndwnd, RUDP_PLDS);

		addSgmBuff(conn->sndsgmbuff, sgm, RUDP_SGM_NACK);

		slideWindowNext(conn->sndwnd, sgm.hdr.plds);

		sendSegment(conn, sgm);

		if (isTimeoutDisarmed(conn->timeout)) {

			DBGPRINT(DEBUG, "TIMEOUT START");

			startTimeout(conn->timeout);
		}

		readStrBuff(conn->sndbuff, strlen(pld));

		free(pld);
	}

	return NULL;
}

static void timeoutHandler(union sigval arg) {
	Connection *conn = (Connection *) arg.sival_ptr;
	SgmBuffElem *curr = NULL;

	DBGPRINT(DEBUG, "INSIDE TIMEOUT");

	lockRead(conn->sndsgmbuff->rwlock);

	curr = conn->sndsgmbuff->head;

	while (curr) {

		if (testSgmBuffElemAttributes(curr, RUDP_SGM_NACK, getTimeoutValue(conn->timeout))) {

			//assert(curr->retrans < 10);

			updateSgmBuffElemAttributes(curr, 1, getTimeoutValue(conn->timeout));

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	unlockRWLock(conn->sndsgmbuff->rwlock);

	if (getSgmBuffSize(conn->sndsgmbuff) > 0)
		startTimeout(conn->timeout);

	DBGPRINT(DEBUG, "ENDOF TIMEOUT");
}

static void sendAck(Connection *conn, const Segment sgm) {
	Segment acksgm;

	acksgm = createSegment(RUDP_ACK, 0, 0, getWindowNext(conn->sndwnd), RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds), NULL);

	sendSegment(conn, acksgm);
}

/* INBOX THREADS */

static void *rcvBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	SgmBuffElem *ackedelem = NULL;
	Segment rcvsgm;
	long double sampleRTT;
	int rcvwndmatch;

	while (1) {

		if (receiveSegment(conn, &rcvsgm) == -1) {

			DBGPRINT(DEBUG, "SEGMENT DROPPED");

			continue;
		}

		rcvwndmatch = matchWindow(conn->rcvwnd, rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			//DBGPRINT(DEBUG, "SEGMENT INSIDE WINDOW: %u", rcvsgm.hdr.seqn);

			if (rcvsgm.hdr.plds != 0) {

				sendAck(conn, rcvsgm);

				if (rcvsgm.hdr.seqn == getWindowBase(conn->rcvwnd)) {

					writeStrBuff(conn->rcvbuff, rcvsgm.pld, rcvsgm.hdr.plds);

					slideWindow(conn->rcvwnd, rcvsgm.hdr.plds);

					//DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwnd->base, conn->rcvwnd->end);

					//char *look = lookStrBuff(conn->rcvbuff, conn->rcvbuff->size);

					//DBGPRINT(DEBUG, "RCVBUFF: %s", look);

					//free(look);

					if (getSgmBuffSize(conn->rcvsgmbuff) > 0)
						signalConditionVariable(conn->rcvsgmbuff->status_cnd);

				} else {

					if (!findSgmBuffSeqn(conn->rcvsgmbuff, rcvsgm.hdr.seqn)) {

						//DBGPRINT(DEBUG, "SEGMENT BUFFERIZED %u", rcvsgm.hdr.seqn);

						addSgmBuff(conn->rcvsgmbuff, rcvsgm, 0);
					}
				}
			}

			if (rcvsgm.hdr.ctrl & RUDP_ACK) {

				if ((ackedelem = findSgmBuffAckn(conn->sndsgmbuff, rcvsgm.hdr.ackn))) {

					setSgmBuffElemStatus(ackedelem, RUDP_SGM_YACK);

					if (ackedelem->segment.hdr.seqn == getWindowBase(conn->sndwnd)) {

						//DBGPRINT(DEBUG, "WNDB ACK %u", conn->sndsgmbuff->head->segment.hdr.seqn);

						while (conn->sndsgmbuff->head) {

							if (getSgmBuffElemStatus(conn->sndsgmbuff->head) != RUDP_SGM_YACK)
								break;

							slideWindow(conn->sndwnd, conn->sndsgmbuff->head->segment.hdr.plds);

							//DBGPRINT(DEBUG, "SNDWND SLIDE: wndb %u wnde %u", conn->sndwnd->base, conn->sndwnd->end);

							sampleRTT = getSgmBuffElemElapsed(ackedelem);

							removeSgmBuff(conn->sndsgmbuff, conn->sndsgmbuff->head);

							updateTimeout(conn->timeout, sampleRTT);
						}
					}
				}
			}			

		} else if (rcvwndmatch == -1) {

			if (rcvsgm.hdr.plds != 0) {

				//DBGPRINT(DEBUG, "SEGMENT BEFORE WINDOW: %u", rcvsgm.hdr.seqn);

				sendAck(conn, rcvsgm);
			}

		} else if (rcvwndmatch == 1){

			//DBGPRINT(DEBUG, "SEGMENT AFTER WINDOW: %u", rcvsgm.hdr.seqn);
		}
	}

	return NULL;
}

static void *rcvSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	SgmBuffElem *curr = NULL;

	while (1) {

		waitStrategicInsertion(conn->rcvsgmbuff);

		while ((curr = findSgmBuffSeqn(conn->rcvsgmbuff, getWindowBase(conn->rcvwnd)))) {

			slideWindow(conn->rcvwnd, curr->segment.hdr.plds);

			writeStrBuff(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

			removeSgmBuff(conn->rcvsgmbuff, curr);

			//DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwnd->base, conn->rcvwnd->end);

			//char *look = lookStrBuff(conn->rcvbuff, conn->rcvbuff->size);

			//DBGPRINT(DEBUG, "RCVBUFF: %s", look);

			//free(look);
		}
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
