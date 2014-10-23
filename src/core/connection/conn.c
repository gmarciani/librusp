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

static void timeoutFunction(Connection *conn);

static void sendAck(Connection *conn, const uint32_t ackn);

/* INBOX */

static void *receiverLoop(void *arg);

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;

	if (!(conn = malloc(sizeof(Connection))))
		ERREXIT("Cannot allocate memory for connection.");

	if ((pthread_rwlock_init(&(conn->state.rwlock), NULL) > 0) |
		(pthread_mutex_init(&(conn->state.mtx), NULL) > 0) |
		(pthread_cond_init(&(conn->state.cnd), NULL) > 0) |
		(pthread_mutex_init(&(conn->sock.mtx), NULL) > 0))
		ERREXIT("Cannot initialize connection sync-block.");

	conn->state.value = RUDP_CON_CLOS;

	conn->sock.fd = -1;

	allocateConnectionInPool(conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {
		
		setConnectionState(conn, RUDP_CON_CLOS);

		if ((pthread_rwlock_destroy(&(conn->state.rwlock)) > 0) |
			(pthread_mutex_destroy(&(conn->state.mtx)) > 0) |
			(pthread_cond_destroy(&(conn->state.cnd)) > 0) |
			(pthread_mutex_destroy(&(conn->sock.mtx)) > 0))
			ERREXIT("Cannot destroy connection sync-block.");

		closeSocket(conn->sock.fd);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);

		cancelThread(conn->sender);

		cancelThread(conn->receiver);

		cancelThread(conn->slider);

		destroyTimeout(&(conn->timeout));

		destroySgmBuff(&(conn->sndsgmbuff));

		destroySgmBuff(&(conn->rcvsgmbuff));

		destroyStrBuff(&(conn->sndusrbuff));

		destroyStrBuff(&(conn->rcvusrbuff));

		destroyWindow(&(conn->sndwnd));

		destroyWindow(&(conn->rcvwnd));

		if ((pthread_rwlock_destroy(&(conn->state.rwlock)) > 0) |
			(pthread_mutex_destroy(&(conn->state.mtx)) > 0) |
			(pthread_cond_destroy(&(conn->state.cnd)) > 0) |
			(pthread_mutex_destroy(&(conn->sock.mtx)) > 0))
			ERREXIT("Cannot destroy connection sync-block.");

		closeSocket(conn->sock.fd);
	}		

	deallocateConnectionInPool(conn);
}

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double sampleRTT) {

	conn->sock.fd = sock;

	setSocketConnected(conn->sock.fd, paddr);

	initializeTimeout(&(conn->timeout), sampleRTT);

	initializeWindow(&(conn->sndwnd),sndwndb, sndwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	initializeWindow(&(conn->rcvwnd), rcvwndb, rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS));

	initializeStrBuff(&(conn->sndusrbuff));

	initializeStrBuff(&(conn->rcvusrbuff));

	initializeSgmBuff(&(conn->sndsgmbuff));

	initializeSgmBuff(&(conn->rcvsgmbuff));

	conn->sender = createThread(senderLoop, conn, THREAD_JOINABLE);

	conn->receiver = createThread(receiverLoop, conn, THREAD_JOINABLE);
}

short getConnectionState(Connection *conn) {
	short state;

	if (pthread_rwlock_rdlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	state = conn->state.value;

	if (pthread_rwlock_unlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return state;
}

void setConnectionState(Connection *conn, const short state) {

	if (pthread_rwlock_wrlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	conn->state.value = state;

	if (pthread_rwlock_unlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	if (pthread_cond_broadcast(&(conn->state.cnd)) > 0)
		ERREXIT("Cannot broadcast condition variable.");
}

/* SEGMENTS I/O */

void sendSegment(Connection *conn, Segment sgm) {
	char ssgm[RUDP_SGMS + 1];
	size_t ssgmsize;

	if (!(sgm.hdr.ctrl & RUDP_ACK)) {

		sgm.hdr.ctrl |= RUDP_ACK;

		sgm.hdr.ackn = getWindowBase(&(conn->rcvwnd));
	}

	ssgmsize = serializeSegment(sgm, ssgm);

	if (pthread_mutex_lock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	writeCSocket(conn->sock.fd, ssgm, ssgmsize);

	DBGFUNC(DEBUG, printOutSegment(getSocketPeer(conn->sock.fd), sgm));

	if (pthread_mutex_unlock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");
}

int receiveSegment(Connection *conn, Segment *sgm) {
	char ssgm[RUDP_SGMS + 1];
	long double timeout;

	if (getRandomBit(DROPRATE)) {

		DBGPRINT(DEBUG, "SEGMENT DROPPPED");

		return 0;
	}

	timeout = getTimeoutValue(&(conn->timeout));

	if (selectSocket(conn->sock.fd, timeout) == 0)
		return 0;

	readCSocket(conn->sock.fd, ssgm, RUDP_SGMS);

	deserializeSegment(ssgm, sgm);

	DBGFUNC(DEBUG, printInSegment(getSocketPeer(conn->sock.fd), *sgm));

	return 1;
}

/* OUTBOX */

static void *senderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	char payload[RUDP_PLDS];
	size_t plds;

	while (1) {

		plds = waitLookMaxStrBuff(&(conn->sndusrbuff), payload, RUDP_PLDS);

		Segment sgm = createSegment((conn->sndusrbuff.size == plds) ? RUDP_PSH : 0, 0, plds, 0, getWindowNext(&(conn->sndwnd)), 0, payload);

		waitWindowSpace(&(conn->sndwnd), plds);

		addSgmBuff(&(conn->sndsgmbuff), sgm, RUDP_SGM_NACK);

		sendSegment(conn, sgm);

		slideWindowNext(&(conn->sndwnd), sgm.hdr.plds);

		DBGPRINT(DEBUG, "SNDWND SLIDENXT: base:%u nxt:%u end:%u SNDBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));

		popStrBuff(&(conn->sndusrbuff), plds);
	}

	return NULL;
}

static void timeoutFunction(Connection *conn) {
	SgmBuffElem *curr = NULL;

	DBGPRINT(DEBUG, "INSIDE TIMEOUT: %LF", getTimeoutValue(&(conn->timeout)));

	if (pthread_rwlock_rdlock(&(conn->sndsgmbuff.rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	curr = conn->sndsgmbuff.head;

	while (curr) {

		if (testSgmBuffElemAttributes(curr, RUDP_SGM_NACK, getTimeoutValue(&(conn->timeout)))) {

			updateSgmBuffElemAttributes(curr, 1, getTimeoutValue(&(conn->timeout)));

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	if (pthread_rwlock_unlock(&(conn->sndsgmbuff.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	DBGPRINT(DEBUG, "ENDOF TIMEOUT");
}

static void sendAck(Connection *conn, const uint32_t ackn) {
	Segment acksgm;

	acksgm = createSegment(RUDP_ACK, 0, 0, 0, getWindowNext(&(conn->sndwnd)), ackn, NULL);

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

			if (getSgmBuffSize(&(conn->sndsgmbuff)) > 0)
				timeoutFunction(conn);

			continue;
		}

		rcvwndmatch = matchWindow(&(conn->rcvwnd), rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			if (rcvsgm.hdr.plds != 0) {

				sendAck(conn, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds));

				if (rcvsgm.hdr.seqn == getWindowBase(&(conn->rcvwnd))) {

					writeStrBuff(&(conn->rcvusrbuff), rcvsgm.pld, rcvsgm.hdr.plds);

					if (rcvsgm.hdr.ctrl & RUDP_PSH)
						allignStrBuffSizeUsr(&(conn->rcvusrbuff));

					slideWindow(&(conn->rcvwnd), rcvsgm.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: base:%u end:%u RCVBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));

					if (getSgmBuffSize(&(conn->rcvsgmbuff)) > 0) {

						SgmBuffElem *curr = NULL;

						while ((curr = findSgmBuffSeqn(&(conn->rcvsgmbuff), getWindowBase(&(conn->rcvwnd))))) {

							Segment sgm = curr->segment;

							removeSgmBuff(&(conn->rcvsgmbuff), curr);

							writeStrBuff(&(conn->rcvusrbuff), sgm.pld, sgm.hdr.plds);

							if (curr->segment.hdr.ctrl & RUDP_PSH)
								allignStrBuffSizeUsr(&(conn->rcvusrbuff));

							slideWindow(&(conn->rcvwnd), sgm.hdr.plds);

							DBGPRINT(DEBUG, "RCVWND SLIDE: base:%u end:%u RCVBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
						}
					}

				} else {

					if (!findSgmBuffSeqn(&(conn->rcvsgmbuff), rcvsgm.hdr.seqn))
						addSgmBuff(&(conn->rcvsgmbuff), rcvsgm, 0);
				}
			}

			if ((rcvsgm.hdr.ctrl & RUDP_ACK) && (ackedelem = findSgmBuffAckn(&(conn->sndsgmbuff), rcvsgm.hdr.ackn))) {

				setSgmBuffElemStatus(ackedelem, RUDP_SGM_YACK);

				if (ackedelem->segment.hdr.seqn == getWindowBase(&(conn->sndwnd))) {

					sampleRTT = getSgmBuffElemElapsed(ackedelem);

					assert(sampleRTT > 0.0);

					while (conn->sndsgmbuff.head) {

						if (getSgmBuffElemStatus(conn->sndsgmbuff.head) != RUDP_SGM_YACK)
							break;

						Segment sgm = conn->sndsgmbuff.head->segment;

						removeSgmBuff(&(conn->sndsgmbuff), conn->sndsgmbuff.head);

						slideWindow(&(conn->sndwnd), sgm.hdr.plds);

						DBGPRINT(DEBUG, "SNDWND SLIDE: base:%u nxt:%u end:%u SNDBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));

					}

					updateTimeout(&(conn->timeout), sampleRTT);
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

	if (pthread_mutex_lock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot lock mutex.");

	conn->connid = CONPOOL_NXTID;

	addElementToList(&CONPOOL, conn);

	CONPOOL_NXTID++;		

	if (pthread_mutex_unlock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot unlock mutex.");

	return conn;
}

void deallocateConnectionInPool(Connection *conn) {

	if (pthread_mutex_lock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot lock mutex.");

	ListElement *curr = CONPOOL.head;

	while (curr) {
		
		if (((Connection *)curr->value)->connid == conn->connid) {
			
			removeElementFromList(&CONPOOL, curr);

			break;
		}

		curr = curr->next;
	}

	if (pthread_mutex_unlock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot unlock mutex.");
}

Connection *getConnectionById(const ConnectionId connid) {
	Connection *conn = NULL;
	ListElement *curr = NULL;

	if (pthread_mutex_lock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot lock mutex.");

	curr = CONPOOL.head;

	while (curr) {

		if (((Connection *) curr->value)->connid == connid) {

			conn = (Connection *) curr->value;

			break;
		}

		curr = curr->next;
	}	

	if (pthread_mutex_unlock(&CONPOOL_MTX) > 0)
		ERREXIT("Cannot unlock mutex.");

	return conn;		
}

/* UTILITY */

void setDropRate(const long double droprate) {
	DROPRATE = droprate;
}

void setConnectionDebugMode(const int dbgmode) {
	DEBUG = dbgmode;
}
