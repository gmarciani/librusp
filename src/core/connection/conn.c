#include "conn.h"

/* DEBUG */

static int DEBUG = 0;

/* DROPRATE */

static double DROPRATE = 0.0;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

static int CONPOOL_NXTID = 0;

static pthread_mutex_t CONPOOL_MTX = PTHREAD_MUTEX_INITIALIZER;

/* OUTBOX */

static void *senderLoop(void *arg);

static void timeoutFunction(void *arg);

static void sendAck(Connection *conn, const uint32_t ackn);

/* INBOX */

static void *receiverLoop(void *arg);

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (!(conn = malloc(sizeof(Connection))))
		ERREXIT("Cannot allocate memory for connection.");

	conn->state.value = RUDP_CON_CLOS;

	conn->state.rwlock = createRWLock();

	conn->state.mtx = createMutex();

	conn->state.cnd = createConditionVariable();

	conn->sock.fd = -1;

	conn->sock.mtx = createMutex();

	allocateConnectionInPool(conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {
		
		setConnectionState(conn, RUDP_CON_CLOS);

		freeRWLock(conn->state.rwlock);

		freeMutex(conn->state.mtx);

		freeConditionVariable(conn->state.cnd);

		freeMutex(conn->sock.mtx);

		closeSocket(conn->sock.fd);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);

		cancelThread(conn->sender);

		cancelThread(conn->receiver);

		cancelThread(conn->slider);

		freeTimeout(conn->timeout);

		freeSgmBuff(conn->sndsgmbuff);

		freeSgmBuff(conn->rcvsgmbuff);

		freeStrBuff(conn->sndbuff);		

		freeStrBuff(conn->rcvbuff);

		freeWindow(conn->sndwnd);

		freeWindow(conn->rcvwnd);

		freeRWLock(conn->state.rwlock);

		freeMutex(conn->state.mtx);

		freeConditionVariable(conn->state.cnd);

		freeMutex(conn->sock.mtx);

		closeSocket(conn->sock.fd);
	}		

	deallocateConnectionInPool(conn);
}

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double sampleRTT) {

	conn->sock.fd = sock;

	setSocketConnected(conn->sock.fd, paddr);

	conn->timeout = createTimeout(sampleRTT);

	conn->sndwnd = createWindow(sndwndb, sndwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	conn->rcvwnd = createWindow(rcvwndb, rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	conn->sndbuff = createStrBuff();

	conn->sndsgmbuff = createSgmBuff();

	conn->rcvbuff = createStrBuff();

	conn->rcvsgmbuff = createSgmBuff();

	conn->sender = createThread(senderLoop, conn, THREAD_JOINABLE);

	conn->receiver = createThread(receiverLoop, conn, THREAD_JOINABLE);
}

short getConnectionState(Connection *conn) {
	short state;

	lockRead(conn->state.rwlock);

	state = conn->state.value;

	unlockRWLock(conn->state.rwlock);

	return state;
}

void setConnectionState(Connection *conn, const short state) {

	lockWrite(conn->state.rwlock);

	conn->state.value = state;

	unlockRWLock(conn->state.rwlock);

	broadcastConditionVariable(conn->state.cnd);
}

/* SEGMENTS I/O */

void sendSegment(Connection *conn, Segment sgm) {
	char *ssgm = NULL;

	if (!(sgm.hdr.ctrl & RUDP_ACK)) {

		sgm.hdr.ctrl |= RUDP_ACK;

		sgm.hdr.ackn = getWindowBase(conn->rcvwnd);
	}

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock.mtx);

	writeConnectedSocket(conn->sock.fd, ssgm);

	DBGFUNC(DEBUG, printOutSegment(getSocketPeer(conn->sock.fd), sgm));

	unlockMutex(conn->sock.mtx);

	free(ssgm);
}

int receiveSegment(Connection *conn, Segment *sgm) {
	char *ssgm = NULL;
	long double timeout;

	if (getRandomBit(DROPRATE)) {

		DBGPRINT(DEBUG, "SEGMENT DROPPPED");

		return 0;
	}

	timeout = getTimeoutValue(conn->timeout);

	if (selectSocket(conn->sock.fd, timeout) == 0)
		return 0;

	ssgm = readConnectedSocket(conn->sock.fd, RUDP_SGMS);

	*sgm = deserializeSegment(ssgm);

	DBGFUNC(DEBUG, printInSegment(getSocketPeer(conn->sock.fd), *sgm));

	free(ssgm);	

	return 1;
}

/* OUTBOX */

static void *senderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	char *pld = NULL;

	while (1) {

		pld = waitLookMaxStrBuff(conn->sndbuff, RUDP_PLDS);

		Segment sgm = createSegment(0, 0, 0, getWindowNext(conn->sndwnd), 0, pld);

		waitWindowSpace(conn->sndwnd, RUDP_PLDS);

		addSgmBuff(conn->sndsgmbuff, sgm, RUDP_SGM_NACK);

		sendSegment(conn, sgm);

		slideWindowNext(conn->sndwnd, sgm.hdr.plds);

		DBGPRINT(DEBUG, "SNDWND SLIDENXT: base:%u nxt:%u end:%u SNDBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(conn->sndwnd), getWindowNext(conn->sndwnd), getWindowEnd(conn->sndwnd), getStrBuffSize(conn->sndbuff), getSgmBuffSize(conn->sndsgmbuff));

		popStrBuff(conn->sndbuff, strlen(pld));

		free(pld);
	}

	return NULL;
}

static void timeoutFunction(void *arg) {
	Connection *conn = (Connection *) arg;
	SgmBuffElem *curr = NULL;

	DBGPRINT(DEBUG, "INSIDE TIMEOUT: %LF", getTimeoutValue(conn->timeout));

	lockRead(conn->sndsgmbuff->rwlock);

	curr = conn->sndsgmbuff->head;

	while (curr) {

		if (testSgmBuffElemAttributes(curr, RUDP_SGM_NACK, getTimeoutValue(conn->timeout))) {

			updateSgmBuffElemAttributes(curr, 1, getTimeoutValue(conn->timeout));

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	unlockRWLock(conn->sndsgmbuff->rwlock);

	DBGPRINT(DEBUG, "ENDOF TIMEOUT");
}

static void sendAck(Connection *conn, const uint32_t ackn) {
	Segment acksgm;

	acksgm = createSegment(RUDP_ACK, 0, 0, getWindowNext(conn->sndwnd), ackn, NULL);

	sendSegment(conn, acksgm);
}

/* INBOX THREADS */

static void *receiverLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	SgmBuffElem *ackedelem = NULL;
	Segment rcvsgm;
	long double sampleRTT;
	int rcvwndmatch;

	while (1) {

		if (!receiveSegment(conn, &rcvsgm)) {

			if (getSgmBuffSize(conn->sndsgmbuff) > 0)
				timeoutFunction(conn);

			continue;
		}

		rcvwndmatch = matchWindow(conn->rcvwnd, rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			if (rcvsgm.hdr.plds != 0) {

				sendAck(conn, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds));

				if (rcvsgm.hdr.seqn == getWindowBase(conn->rcvwnd)) {

					writeStrBuff(conn->rcvbuff, rcvsgm.pld, rcvsgm.hdr.plds);

					slideWindow(conn->rcvwnd, rcvsgm.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: base:%u end:%u RCVBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(conn->rcvwnd), getWindowEnd(conn->rcvwnd), getStrBuffSize(conn->rcvbuff), getSgmBuffSize(conn->rcvsgmbuff));

					SgmBuffElem *curr = NULL;

					while ((curr = findSgmBuffSeqn(conn->rcvsgmbuff, getWindowBase(conn->rcvwnd)))) {

						Segment sgm = curr->segment;

						removeSgmBuff(conn->rcvsgmbuff, curr);

						writeStrBuff(conn->rcvbuff, sgm.pld, sgm.hdr.plds);

						slideWindow(conn->rcvwnd, sgm.hdr.plds);

						DBGPRINT(DEBUG, "RCVWND SLIDE: base:%u end:%u RCVBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(conn->rcvwnd), getWindowEnd(conn->rcvwnd), getStrBuffSize(conn->rcvbuff), getSgmBuffSize(conn->rcvsgmbuff));
					}

				} else {

					if (!findSgmBuffSeqn(conn->rcvsgmbuff, rcvsgm.hdr.seqn))
						addSgmBuff(conn->rcvsgmbuff, rcvsgm, 0);
				}
			}

			if ((rcvsgm.hdr.ctrl & RUDP_ACK) && (ackedelem = findSgmBuffAckn(conn->sndsgmbuff, rcvsgm.hdr.ackn))) {

				setSgmBuffElemStatus(ackedelem, RUDP_SGM_YACK);

				if (ackedelem->segment.hdr.seqn == getWindowBase(conn->sndwnd)) {

					sampleRTT = getSgmBuffElemElapsed(ackedelem);

					assert(sampleRTT > 0.0);

					while (conn->sndsgmbuff->head) {

						if (getSgmBuffElemStatus(conn->sndsgmbuff->head) != RUDP_SGM_YACK)
							break;

						Segment sgm = conn->sndsgmbuff->head->segment;

						removeSgmBuff(conn->sndsgmbuff, conn->sndsgmbuff->head);

						slideWindow(conn->sndwnd, sgm.hdr.plds);

						DBGPRINT(DEBUG, "SNDWND SLIDE: base:%u nxt:%u end:%u SNDBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(conn->sndwnd), getWindowNext(conn->sndwnd), getWindowEnd(conn->sndwnd), getStrBuffSize(conn->sndbuff), getSgmBuffSize(conn->sndsgmbuff));

					}

					updateTimeout(conn->timeout, sampleRTT);
				}
			}

		} else if (rcvwndmatch == -1 && rcvsgm.hdr.plds != 0) {

			sendAck(conn,  RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds));
		} else {
			DBGPRINT(DEBUG, "SEGMENT NOT CONSIDERED %u", rcvsgm.hdr.seqn);
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

/* UTILITY */

void setDropRate(const long double droprate) {
	DROPRATE = droprate;
}

void setConnectionDebugMode(const int dbgmode) {
	DEBUG = dbgmode;
}
