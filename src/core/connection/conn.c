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

//static void *timeoutManagerLoop(void *arg);

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

	setConnectionState(conn, RUDP_CON_CLOS);

	allocateConnectionInPool(conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (getConnectionState(conn) < RUDP_CON_ESTA) {
		
		setConnectionState(conn, RUDP_CON_CLOS);

		destroyMutex(conn->state_mtx);

		destroyConditionVariable(conn->state_cnd);	

		destroyMutex(conn->sock_mtx);		

		closeSocket(conn->sock);

	} else {

		setConnectionState(conn, RUDP_CON_CLOS);

		cancelThread(conn->sndbufferizer);

		cancelThread(conn->sndslider);

		cancelThread(conn->rcvbufferizer);

		cancelThread(conn->rcvslider);

		freeTimeout(conn->timeout);

		freeSgmBuff(conn->sndsgmbuff);

		freeSgmBuff(conn->rcvsgmbuff);

		freeStrBuff(conn->sndbuff);		

		freeStrBuff(conn->rcvbuff);

		freeWindow(conn->sndwnd);

		freeWindow(conn->rcvwnd);

		destroyMutex(conn->state_mtx);

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

	//conn->sndslider = createThread(sndSliderLoop, conn, THREAD_JOINABLE);

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

/* SEGMENTS I/O */

void sendSegment(Connection *conn, Segment sgm) {
	char *ssgm = NULL;

	if (!(sgm.hdr.ctrl & RUDP_ACK)) {

		sgm.hdr.ctrl |= RUDP_ACK;

		sgm.hdr.ackn = conn->rcvwnd->base;
	}

	ssgm = serializeSegment(sgm);

	lockMutex(conn->sock_mtx);

	writeConnectedSocket(conn->sock, ssgm);

	DBGFUNC(DEBUG, printOutSegment(getSocketPeer(conn->sock), sgm));	

	free(ssgm);

	unlockMutex(conn->sock_mtx);
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
	SgmBuffElem *curr = NULL;
	char *pld = NULL;

	while (1) {

		lockMutex(conn->sndbuff->mtx);

		while (conn->sndbuff->size == 0)
			waitConditionVariable(conn->sndbuff->insert_cnd, conn->sndbuff->mtx);

		lockMutex(conn->sndsgmbuff->mtx);

		while (getWindowSpace(conn->sndwnd) < RUDP_PLDS) {

			DBGPRINT(DEBUG, "NO SPACE IN SNDWND: sndwndb:%u sndnext:%u sdwndend:%u", conn->sndwnd->base, conn->sndwnd->next, conn->sndwnd->end);

			waitConditionVariable(conn->sndsgmbuff->remove_cnd, conn->sndsgmbuff->mtx);
		}

		pld = readStrBuff(conn->sndbuff, RUDP_PLDS);

		DBGPRINT(DEBUG, "READ BUFFER (%s|%zu): sndwndb:%u sndnext:%u sdwndend:%u", pld, strlen(pld), conn->sndwnd->base, conn->sndwnd->next, conn->sndwnd->end);

		unlockMutex(conn->sndbuff->mtx);

		Segment sgm = createSegment(0, 0, 0, conn->sndwnd->next, 0, pld);

		free(pld);

		curr = addSgmBuff(conn->sndsgmbuff, sgm);

		curr->status = RUDP_SGM_NACK;

		clock_gettime(CLOCK_MONOTONIC, &(curr->time));

		unlockMutex(conn->sndsgmbuff->mtx);

		signalConditionVariable(conn->sndsgmbuff->insert_cnd);

		slideWindowNext(conn->sndwnd, sgm.hdr.plds);

		sendSegment(conn, sgm);

		if (isTimeoutDisarmed(conn->timeout)) {

			DBGPRINT(DEBUG, "TIMEOUT START");

			startTimeout(conn->timeout);
		}
	}

	return NULL;
}

static void *sndSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;

	while (1) {

		lockMutex(conn->sndsgmbuff->mtx);	

		while (conn->sndsgmbuff->size == 0)
			waitConditionVariable(conn->sndsgmbuff->insert_cnd, conn->sndsgmbuff->mtx);

		while (conn->sndsgmbuff->head->status != RUDP_SGM_YACK)
			waitConditionVariable(conn->sndsgmbuff->status_cnd, conn->sndsgmbuff->mtx);

		DBGPRINT(DEBUG, "WNDB ACK DETECTED %u", conn->sndsgmbuff->head->segment.hdr.seqn);

		slideWindow(conn->sndwnd, conn->sndsgmbuff->head->segment.hdr.plds);

		DBGPRINT(DEBUG, "SNDWND SLIDE: wndb %u wnde %u", conn->sndwnd->base, conn->sndwnd->end);

		removeSgmBuff(conn->sndsgmbuff, conn->sndsgmbuff->head);

		if (conn->sndsgmbuff->size == 0) {

			DBGPRINT(DEBUG, "TIMEOUT STOP");

			stopTimeout(conn->timeout);

		} else {

			DBGPRINT(DEBUG, "TIMEOUT RESTART");

			startTimeout(conn->timeout);
		}

		unlockMutex(conn->sndsgmbuff->mtx);	

		broadcastConditionVariable(conn->sndsgmbuff->remove_cnd);
	}	

	return NULL;
}

static void timeoutHandler(union sigval arg) {
	Connection *conn = (Connection *) arg.sival_ptr;
	struct timespec now;
	SgmBuffElem *curr = NULL;

	if (timer_getoverrun(conn->timeout->timer) > 0) {
		DBGPRINT(DEBUG, "TIMER OVERRUN DETECTED");
		return;
	}

	DBGPRINT(DEBUG, "INSIDE TIMEOUT");

	lockMutex(conn->sndsgmbuff->mtx);

	curr = conn->sndsgmbuff->head;

	while (curr) {

		now = getTimestamp();

		DBGPRINT(DEBUG, "TIMEOUT CHECK %u: status %d elapsed %LF delay %LF retrans %ld", curr->segment.hdr.seqn, curr->status, getElapsed(curr->time, now), curr->delay, curr->retrans);

		if ((matchWindow(conn->sndwnd, curr->segment.hdr.seqn) == 0) &
				(curr->status == RUDP_SGM_NACK) &
				((getElapsed(curr->time, now) - curr->delay) > getTimeoutValue(conn->timeout))) {

			//assert(curr->retrans < 10);

			curr->retrans++;

			curr->delay = getTimeoutValue(conn->timeout);

			curr->time = getTimestamp();

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	unlockMutex(conn->sndsgmbuff->mtx);

	if (conn->sndsgmbuff->size > 0)
		startTimeout(conn->timeout);

	DBGPRINT(DEBUG, "ENDOF TIMEOUT");
}

/* INBOX THREADS */

static void *rcvBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm, acksgm;
	struct timespec now;
	long double sampleRTT;
	int rcvwndmatch;

	while (1) {

		if (receiveSegment(conn, &rcvsgm) == -1) {

			DBGPRINT(DEBUG, "SEGMENT DROPPED");

			continue;
		}

		rcvwndmatch = matchWindow(conn->rcvwnd, rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			DBGPRINT(DEBUG, "SEGMENT INSIDE WINDOW: %u", rcvsgm.hdr.seqn);

			if (rcvsgm.hdr.plds != 0) {

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndwnd->next, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

				sendSegment(conn, acksgm);

				if (rcvsgm.hdr.seqn == conn->rcvwnd->base) {

					lockMutex(conn->rcvbuff->mtx);

					writeStrBuff(conn->rcvbuff, rcvsgm.pld, rcvsgm.hdr.plds);

					unlockMutex(conn->rcvbuff->mtx);

					slideWindow(conn->rcvwnd, rcvsgm.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwnd->base, conn->rcvwnd->end);

					char *look = lookStrBuff(conn->rcvbuff, conn->rcvbuff->size);

					DBGPRINT(DEBUG, "RCVBUFF: %s", look);

					free(look);

					signalConditionVariable(conn->rcvbuff->insert_cnd);

					if (conn->rcvsgmbuff->size > 0)
						signalConditionVariable(conn->rcvsgmbuff->status_cnd);

				} else {

					lockMutex(conn->rcvsgmbuff->mtx);

					if (!findSgmBuffSeqn(conn->rcvsgmbuff, rcvsgm.hdr.seqn)) {

						DBGPRINT(DEBUG, "SEGMENT BUFFERIZED %u", rcvsgm.hdr.seqn);

						addSgmBuff(conn->rcvsgmbuff, rcvsgm);

						signalConditionVariable(conn->rcvsgmbuff->insert_cnd);
					}

					unlockMutex(conn->rcvsgmbuff->mtx);
				}
			}

			if (rcvsgm.hdr.ctrl & RUDP_ACK) {
				struct timespec sendtime;
				int acked, wndbacked = 0;

				lockMutex(conn->sndsgmbuff->mtx);

				SgmBuffElem *ackedelem = findSgmBuffAckn(conn->sndsgmbuff, rcvsgm.hdr.ackn);

				if (ackedelem) {

					ackedelem->status = RUDP_SGM_YACK;

					sendtime = ackedelem->time;

					if (ackedelem->segment.hdr.seqn == conn->sndwnd->base) {

						DBGPRINT(DEBUG, "WNDB ACK %u", conn->sndsgmbuff->head->segment.hdr.seqn);

						while (conn->sndsgmbuff->head) {

							if (conn->sndsgmbuff->head->status != RUDP_SGM_YACK)
								break;

							slideWindow(conn->sndwnd, conn->sndsgmbuff->head->segment.hdr.plds);

							DBGPRINT(DEBUG, "SNDWND SLIDE: wndb %u wnde %u", conn->sndwnd->base, conn->sndwnd->end);

							removeSgmBuff(conn->sndsgmbuff, conn->sndsgmbuff->head);

							wndbacked = 1;
						}
					}

					acked = 1;
				}

				unlockMutex(conn->sndsgmbuff->mtx);

				if (wndbacked)
					broadcastConditionVariable(conn->sndsgmbuff->remove_cnd);

				if (acked) {

					now = getTimestamp();

					sampleRTT = getElapsed(sendtime, now);

					updateTimeout(conn->timeout, sampleRTT);
				}
			}			

		} else if (rcvwndmatch == -1) {

			if (rcvsgm.hdr.plds != 0) {

				DBGPRINT(DEBUG, "SEGMENT BEFORE WINDOW: %u", rcvsgm.hdr.seqn);

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndwnd->next, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

				sendSegment(conn, acksgm);
			}

		} else if (rcvwndmatch == 1){

			DBGPRINT(DEBUG, "SEGMENT AFTER WINDOW: %u", rcvsgm.hdr.seqn);
		}
	}

	return NULL;
}

static void *rcvSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;

	while (1) {

		lockMutex(conn->rcvsgmbuff->mtx);

		while (conn->rcvsgmbuff->size == 0)
			waitConditionVariable(conn->rcvsgmbuff->insert_cnd, conn->rcvsgmbuff->mtx);

		waitConditionVariable(conn->rcvsgmbuff->status_cnd, conn->rcvsgmbuff->mtx);

		int flag = 1;

		while (flag) {

			flag = 0;

			SgmBuffElem *curr = conn->rcvsgmbuff->head;

			while (curr) {

				if (curr->segment.hdr.seqn == conn->rcvwnd->base) {

					lockMutex(conn->rcvbuff->mtx);

					writeStrBuff(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

					unlockMutex(conn->rcvbuff->mtx);

					signalConditionVariable(conn->rcvbuff->insert_cnd);

					removeSgmBuff(conn->rcvsgmbuff, curr);

					slideWindow(conn->rcvwnd, curr->segment.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwnd->base, conn->rcvwnd->end);

					flag = 1;

					char *look = lookStrBuff(conn->rcvbuff, conn->rcvbuff->size);

					DBGPRINT(DEBUG, "RCVBUFF: %s", look);

					free(look);

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
