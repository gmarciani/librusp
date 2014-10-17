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

	conn->sndwnd_mtx = createMutex();

	conn->rcvwnd_mtx = createMutex();

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

		freeTimer(conn->timer);

		cancelThread(conn->sndbufferizer);

		cancelThread(conn->sndslider);

		cancelThread(conn->rcvbufferizer);

		cancelThread(conn->rcvslider);

		freeSgmBuff(conn->sndsgmbuff);

		freeSgmBuff(conn->rcvsgmbuff);

		freeStrBuff(conn->sndbuff);		

		freeStrBuff(conn->rcvbuff);

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

	conn->timer = createTimer(timeoutHandler, conn);


	conn->sndwndb = sndwndb;

	conn->sndwnde = sndwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->sndnext = sndwndb;

	conn->sndbuff = createStrBuff();

	conn->sndsgmbuff = createSgmBuff();

	conn->sndbufferizer = createThread(sndBufferizerLoop, conn, THREAD_JOINABLE);

	conn->sndslider = createThread(sndSliderLoop, conn, THREAD_JOINABLE);


	conn->rcvwndb = rcvwndb;

	conn->rcvwnde = rcvwndb + (RUDP_PLDS * RUDP_CON_WNDS);

	conn->rcvbuff = createStrBuff();

	conn->rcvsgmbuff = createSgmBuff();

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

void sendSegment(Connection *conn, Segment sgm) {
	char *ssgm = NULL;

	if (!(sgm.hdr.ctrl & RUDP_ACK)) {

		sgm.hdr.ctrl |= RUDP_ACK;

		sgm.hdr.ackn = conn->rcvwndb;
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
	long double timeout;
	char *pld = NULL;
	char *timestamp = NULL;

	while (1) {

		lockMutex(conn->sndbuff->mtx);

		while (conn->sndbuff->size == 0)
			waitConditionVariable(conn->sndbuff->insert_cnd, conn->sndbuff->mtx);

		lockMutex(conn->sndsgmbuff->mtx);

		DBGPRINT(DEBUG, "READING BUFFER: size %zu wndspace: %u", conn->sndbuff->size, (conn->sndwnde - conn->sndnext));

		while ((conn->sndwnde - conn->sndnext) < RUDP_PLDS) {

			DBGPRINT(DEBUG, "NO SPACE IN WINDOW");

			waitConditionVariable(conn->sndsgmbuff->remove_cnd, conn->sndsgmbuff->mtx);
		}

		pld = readStrBuff(conn->sndbuff, RUDP_PLDS);

		unlockMutex(conn->sndbuff->mtx);

		Segment sgm = createSegment(0, 0, 0, conn->sndnext, 0, pld);

		DBGPRINT(DEBUG, "SEGMENTIZED %s", pld);

		free(pld);

		curr = addSgmBuff(conn->sndsgmbuff, sgm);

		curr->status = RUDP_SGM_NACK;

		clock_gettime(CLOCK_MONOTONIC, &(curr->time));

		unlockMutex(conn->sndsgmbuff->mtx);

		signalConditionVariable(conn->sndsgmbuff->insert_cnd);

		conn->sndnext = RUDP_NXTSEQN(sgm.hdr.seqn, sgm.hdr.plds);

		sendSegment(conn, sgm);

		if (isTimerDisarmed(conn->timer)) {

			timeout = getTimeout(conn);

			timestamp = getTime();

			DBGPRINT(DEBUG, "%s TIMEOUT START: %LF", timestamp, timeout);

			free(timestamp);

			setTimer(conn->timer, timeout, 0.0);
		}
	}

	return NULL;
}

static void *sndSliderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	long double timeout;
	char *timestamp = NULL;

	while (1) {

		lockMutex(conn->sndsgmbuff->mtx);	

		while (conn->sndsgmbuff->size == 0)
			waitConditionVariable(conn->sndsgmbuff->insert_cnd, conn->sndsgmbuff->mtx);

		while (conn->sndsgmbuff->head->status != RUDP_SGM_YACK)
			waitConditionVariable(conn->sndsgmbuff->status_cnd, conn->sndsgmbuff->mtx);

		DBGPRINT(DEBUG, "WNDB ACKED %u", conn->sndsgmbuff->head->segment.hdr.seqn);

		conn->sndwndb = RUDP_NXTSEQN(conn->sndwndb, conn->sndsgmbuff->head->segment.hdr.plds);

		conn->sndwnde = RUDP_NXTSEQN(conn->sndwnde, conn->sndsgmbuff->head->segment.hdr.plds);

		DBGPRINT(DEBUG, "SNDWND SLIDE: wndb %u wnde %u", conn->sndwndb, conn->sndwnde);

		removeSgmBuff(conn->sndsgmbuff, conn->sndsgmbuff->head);

		if (conn->sndsgmbuff->size == 0) {

			timestamp = getTime();

			DBGPRINT(DEBUG, "%s TIMEOUT STOP", timestamp);

			free(timestamp);

			setTimer(conn->timer, 0.0, 0.0);

		} else {

			timeout = getTimeout(conn);

			timestamp = getTime();

			DBGPRINT(DEBUG, "%s TIMEOUT RESTART: %LF", timestamp, timeout);

			free(timestamp);

			setTimer(conn->timer, timeout, 0.0);
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
	char *timestamp = NULL;

	if (timer_getoverrun(conn->timer) > 0) {
		DBGPRINT(DEBUG, "TIMER OVERRUN DETECTED");
		return;
	}

	setTimer(conn->timer, 0.0, 0.0);

	timestamp = getTime();

	DBGPRINT(DEBUG, "%s INSIDE TIMEOUT", timestamp);

	free(timestamp);

	lockMutex(conn->sndsgmbuff->mtx);

	curr = conn->sndsgmbuff->head;

	while (curr) {

		clock_gettime(CLOCK_MONOTONIC, &now);

		DBGPRINT(DEBUG, "TIMEOUT CHECK %u: status %d elapsed %LF delay %LF retrans %ld", curr->segment.hdr.seqn, curr->status, getElapsed(curr->time, now), curr->delay, curr->retrans);

		if ((curr->status == RUDP_SGM_NACK) & ((getElapsed(curr->time, now) - curr->delay) > getTimeout(conn))) {

			assert(curr->retrans <= 20);

			curr->retrans++;

			curr->delay = getTimeout(conn);

			clock_gettime(CLOCK_MONOTONIC, &(curr->time));

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	if (conn->sndsgmbuff->size > 0)
		setTimer(conn->timer, getTimeout(conn), 0.0);

	unlockMutex(conn->sndsgmbuff->mtx);

	DBGPRINT(DEBUG, "ENDOF TIMEOUT");
}

/* INBOX THREADS */

static void *rcvBufferizerLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm, acksgm;
	struct timespec now;
	long double sampleRTT;
	int rcvwndmatch;
	char *timestamp = NULL;

	while (1) {

		if (receiveSegment(conn, &rcvsgm) == -1) {

			DBGPRINT(DEBUG, "SEGMENT DROPPED");

			continue;
		}

		rcvwndmatch = matchSequenceAgainstWindow(conn->rcvwndb, conn->rcvwnde, rcvsgm.hdr.seqn);

		if (rcvwndmatch == 0) {

			DBGPRINT(DEBUG, "SEGMENT INSIDE WINDOW: %u", rcvsgm.hdr.seqn);

			if (rcvsgm.hdr.plds != 0) {

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

				sendSegment(conn, acksgm);

				if (rcvsgm.hdr.seqn == conn->rcvwndb) {

					lockMutex(conn->rcvbuff->mtx);

					writeStrBuff(conn->rcvbuff, rcvsgm.pld, rcvsgm.hdr.plds);

					unlockMutex(conn->rcvbuff->mtx);

					conn->rcvwndb = RUDP_NXTSEQN(conn->rcvwndb, rcvsgm.hdr.plds);

					conn->rcvwnde = RUDP_NXTSEQN(conn->rcvwnde, rcvsgm.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwndb, conn->rcvwnde);

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

				lockMutex(conn->sndsgmbuff->mtx);

				SgmBuffElem *ackedelem = findSgmBuffAckn(conn->sndsgmbuff, rcvsgm.hdr.ackn);

				if (ackedelem) {

					timestamp = getTime();

					ackedelem->status = RUDP_SGM_YACK;

					clock_gettime(CLOCK_MONOTONIC, &(now));

					sampleRTT = getElapsed(conn->sndsgmbuff->head->time, now);

					setTimeout(conn, sampleRTT);

					DBGPRINT(DEBUG, "%s ACK %u sample %LF new timeout %LF", timestamp, rcvsgm.hdr.ackn, sampleRTT, conn->timeo);

					signalConditionVariable(conn->sndsgmbuff->status_cnd);

					free(timestamp);
				}

				unlockMutex(conn->sndsgmbuff->mtx);
			}			

		} else if (rcvwndmatch == -1) {

			if (rcvsgm.hdr.plds != 0) {

				DBGPRINT(DEBUG, "SEGMENT BEFORE WINDOW: %u", rcvsgm.hdr.seqn);

				acksgm = createSegment(RUDP_ACK, 0, 0, conn->sndnext, RUDP_NXTSEQN(rcvsgm.hdr.seqn, rcvsgm.hdr.plds), NULL);

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

				if (curr->segment.hdr.seqn == conn->rcvwndb) {

					lockMutex(conn->rcvbuff->mtx);

					writeStrBuff(conn->rcvbuff, curr->segment.pld, curr->segment.hdr.plds);

					unlockMutex(conn->rcvbuff->mtx);

					signalConditionVariable(conn->rcvbuff->insert_cnd);

					removeSgmBuff(conn->rcvsgmbuff, curr);

					conn->rcvwndb = RUDP_NXTSEQN(conn->rcvwndb, curr->segment.hdr.plds);

					conn->rcvwnde = RUDP_NXTSEQN(conn->rcvwnde, curr->segment.hdr.plds);

					DBGPRINT(DEBUG, "RCVWND SLIDE: wndb %u wnde %u", conn->rcvwndb, conn->rcvwnde);

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
