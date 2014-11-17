#include "conn.h"

/* GLOBAL VARIABLES */

int RUSP_DEBUG = 0;

double RUSP_DROP = 0.0;

/* CONNECTIONS POOL */

static List CONPOOL = LIST_INITIALIZER;

/* SOCKET THREADS */

static void *senderLoop(void *arg);

static void *receiverLoop(void *arg);

static void *timeWaitFunction(void *arg);

/* SOCKET THREADS SUBPROCEDURES */

static void timeoutFunction(Connection *conn);

static void processRcvWndBase(Connection *conn, const Segment rcvsgm);

static void submitSACK(Connection *conn, const uint32_t ackn);

static void submitCACK(Connection *conn, const uint32_t ackn);

static void sendSACK(Connection *conn, const uint32_t ackn);

static void sendCACK(Connection *conn, const uint32_t ackn);

static void cleanupFunction(void *arg);

/* SEGMENT I/O */

static int sendSegment(Connection *conn, Segment sgm);

static int receiveSegment(Connection *conn, Segment *sgm);

/* CONNECTION */

Connection *createConnection(void) {
	Connection *conn = NULL;
	ListElement *elem = NULL;

	if (!(conn = malloc(sizeof(Connection))))
		ERREXIT("Cannot allocate memory for connection.");

	pthread_rwlock_init(&(conn->state.rwlock), NULL);

	pthread_mutex_init(&(conn->sock.mtx), NULL);

	conn->state.value = RUSP_CLOSED;

	conn->sock.fd = -1;

	elem = CONPOOL.head;

	while (elem) {

		if (getConnectionState((Connection *) elem->value) == RUSP_TIMEWT) {

			usleep(RUSP_TIMEWTTM);

			elem = CONPOOL.head;

		} else {

			elem = elem->next;
		}
	}

	conn->connid = (ConnectionId) addElementToList(&CONPOOL, conn);

	return conn;
}

void destroyConnection(Connection *conn) {

	if (pthread_rwlock_destroy(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot destroy connection state read-write lock.");

	if (pthread_mutex_destroy(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot destroy connection socket mutex.");

	destroyTimeout(&(conn->timeout));

	destroyWindow(&(conn->sndwnd));

	destroyWindow(&(conn->rcvwnd));

	destroySgmBuff(&(conn->sndsgmbuff));

	destroySgmBuff(&(conn->rcvsgmbuff));

	destroyStrBuff(&(conn->sndusrbuff));

	destroyStrBuff(&(conn->rcvusrbuff));

	close(conn->sock.fd);

	removeElementFromList(&CONPOOL, conn->connid);
}

void setupConnection(Connection *conn, const int sock, const struct sockaddr_in paddr, const uint32_t sndwndb, const uint32_t rcvwndb, const long double sampleRTT) {

	conn->sock.fd = sock;

	setSocketConnected(conn->sock.fd, paddr);

	initializeTimeout(&(conn->timeout), sampleRTT);

	initializeWindow(&(conn->sndwnd),sndwndb, sndwndb + (RUSP_PLDS * RUSP_WNDS));

	initializeWindow(&(conn->rcvwnd), rcvwndb, rcvwndb + (RUSP_PLDS * RUSP_WNDS));

	initializeStrBuff(&(conn->sndusrbuff));

	initializeStrBuff(&(conn->rcvusrbuff));

	initializeSgmBuff(&(conn->sndsgmbuff));

	initializeSgmBuff(&(conn->rcvsgmbuff));

	setConnectionState(conn, RUSP_ESTABL);

	conn->sender = createThread(senderLoop, conn, THREAD_JOINABLE);

	conn->receiver = createThread(receiverLoop, conn, THREAD_JOINABLE);
}

int getConnectionState(Connection *conn) {
	int state;

	if (pthread_rwlock_rdlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	state = conn->state.value;

	if (pthread_rwlock_unlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return state;
}

void setConnectionState(Connection *conn, const int state) {

	if (pthread_rwlock_wrlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	DBGPRINT(RUSP_DEBUG, "STATE: %d -> %d", conn->state.value, state);

	conn->state.value = state;

	if (pthread_rwlock_unlock(&(conn->state.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");
}

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {

	if (getConnectionState(conn) != RUSP_CLOSED)
		ERREXIT("Cannot setup listening connection: connection not closed.");

	conn->sock.fd = openSocket();

	setSocketReusable(conn->sock.fd);

	bindSocket(conn->sock.fd, &laddr);

	setConnectionState(conn, RUSP_LISTEN);
}

/* SYNCHRONIZATION */

int activeOpen(Connection *conn, const struct sockaddr_in laddr) {
	Segment syn, synack, acksynack;
	char ssyn[RUSP_SGMS + 1], ssynack[RUSP_SGMS + 1], sacksynack[RUSP_SGMS + 1];
	int asock, synretrans;
	struct sockaddr_in aaddr;
	struct timespec start, end;
	long double sampleRTT;

	if (getConnectionState(conn) != RUSP_CLOSED)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	syn = createSegment(RUSP_SYN, 0, 0, 0, NULL);

	serializeSegment(syn, ssyn);

	for (synretrans = 0; synretrans < RUSP_SYN_RETR; synretrans++) {

		clock_gettime(CLOCK_MONOTONIC, &start);

		writeUSocket(asock, laddr, ssyn, strlen(ssyn));

		DBGFUNC(RUSP_DEBUG, printOutSegment(laddr, syn));

		setConnectionState(conn, RUSP_SYNSND);

		if (!selectSocket(asock, RUSP_SAMPLRTT))
			continue;

		readUSocket(asock, &aaddr, ssynack, RUSP_SGMS);

		clock_gettime(CLOCK_MONOTONIC, &end);

		sampleRTT = getElapsed(start, end);

		deserializeSegment(ssynack, &synack);

		DBGFUNC(RUSP_DEBUG, printInSegment(aaddr, synack));

		if ((synack.hdr.ctrl == (RUSP_SYN | RUSP_SACK)) &
			(synack.hdr.ackn == RUSP_NXTSEQN(syn.hdr.seqn, 1))) {

			setConnectionState(conn, RUSP_SYNRCV);

			acksynack = createSegment(RUSP_SACK, 0, RUSP_NXTSEQN(syn.hdr.seqn, 1), RUSP_NXTSEQN(synack.hdr.seqn, 1), NULL);

			serializeSegment(acksynack, sacksynack);

			writeUSocket(asock, aaddr, sacksynack, strlen(sacksynack));

			DBGFUNC(RUSP_DEBUG, printOutSegment(aaddr, acksynack));

			setupConnection(conn, asock, aaddr, acksynack.hdr.seqn, acksynack.hdr.ackn, sampleRTT);

			return conn->connid;
		}
	}

	closeSocket(asock);

	setConnectionState(conn, RUSP_CLOSED);

	return -1;
}

ConnectionId passiveOpen(Connection *lconn) {
	Connection *aconn = NULL;
	Segment syn, synack, acksynack;
	char ssyn[RUSP_SGMS + 1], ssynack[RUSP_SGMS + 1], sacksynack[RUSP_SGMS + 1];
	int asock, synackretrans;
	struct sockaddr_in caddr;
	struct timespec start, end;
	long double sampleRTT;

	while (getConnectionState(lconn) == RUSP_LISTEN) {

		readUSocket(lconn->sock.fd, &caddr, ssyn, RUSP_SGMS);

		deserializeSegment(ssyn, &syn);

		DBGFUNC(RUSP_DEBUG, printInSegment(caddr, syn));

		if (syn.hdr.ctrl != RUSP_SYN)
			continue;

		setConnectionState(lconn, RUSP_SYNRCV);

		asock = openSocket();

		synack = createSegment(RUSP_SYN | RUSP_SACK, 0, 10, RUSP_NXTSEQN(syn.hdr.seqn, 1), NULL);

		serializeSegment(synack, ssynack);

		for (synackretrans = 0; synackretrans < RUSP_RETR; synackretrans++) {

			clock_gettime(CLOCK_MONOTONIC, &start);

			writeUSocket(asock, caddr, ssynack, strlen(ssynack));

			DBGFUNC(RUSP_DEBUG, printOutSegment(caddr, synack));

			setConnectionState(lconn, RUSP_SYNSND);

			if (!selectSocket(asock, RUSP_SAMPLRTT))
				continue;

			readUSocket(asock, &caddr, sacksynack, RUSP_SGMS);

			clock_gettime(CLOCK_MONOTONIC, &end);

			sampleRTT = getElapsed(start, end);

			deserializeSegment(sacksynack, &acksynack);

			DBGFUNC(RUSP_DEBUG, printInSegment(caddr, acksynack));

			if ((acksynack.hdr.ctrl == RUSP_SACK) &
				(acksynack.hdr.seqn == synack.hdr.ackn) &
				(acksynack.hdr.ackn == RUSP_NXTSEQN(synack.hdr.seqn, 1))) {

				aconn = createConnection();

				setConnectionState(aconn, RUSP_SYNSND);

				setupConnection(aconn, asock, caddr, acksynack.hdr.ackn, acksynack.hdr.seqn, sampleRTT);

				setConnectionState(lconn, RUSP_LISTEN);

				return aconn->connid;
			}
		}

		closeSocket(asock);

		setConnectionState(lconn, RUSP_LISTEN);
	}

	return -1;
}

/* DESYNCHRONIZATION */

void activeClose(Connection *conn) {
	Segment fin;

	cancelThread(conn->sender);

	joinThread(conn->sender);

	fin = createSegment(RUSP_FIN, 0, getWindowNext(&(conn->sndwnd)), 0, NULL);

	addSgmBuff(&(conn->sndsgmbuff), fin, RUSP_NACK);

	setConnectionState(conn, RUSP_FINWT1);

	sendSegment(conn, fin);

	slideWindowNext(&(conn->sndwnd), 1);

	DBGPRINT(RUSP_DEBUG, "SND (NXT): base:%u nxt:%u end:%u SNDUSRBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));

	joinThread(conn->receiver);
}

void passiveClose(Connection *conn) {
	Segment fin;

	cancelThread(conn->sender);

	joinThread(conn->sender);

	fin = createSegment(RUSP_FIN, 0, getWindowNext(&(conn->sndwnd)), 0, NULL);

	addSgmBuff(&(conn->sndsgmbuff), fin, RUSP_NACK);

	setConnectionState(conn, RUSP_LSTACK);

	sendSegment(conn, fin);

	slideWindowNext(&(conn->sndwnd), 1);

	DBGPRINT(RUSP_DEBUG, "SND (NXT): base:%u nxt:%u end:%u SNDUSRBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));

	joinThread(conn->receiver);

	destroyConnection(conn);
}

/* SOCKET THREADS */

static void *senderLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	char payload[RUSP_PLDS];
	size_t plds;

	pthread_cleanup_push(cleanupFunction, &(conn->sndusrbuff.mtx));

	while (1) {

		plds = waitLookMaxStrBuff(&(conn->sndusrbuff), payload, RUSP_PLDS);

		Segment sgm = createSegment((conn->sndusrbuff.size == plds) ? RUSP_PSH : RUSP_NUL, plds, getWindowNext(&(conn->sndwnd)), 0, payload);

		waitWindowSpace(&(conn->sndwnd), plds);

		addSgmBuff(&(conn->sndsgmbuff), sgm, RUSP_NACK);

		sendSegment(conn, sgm);

		slideWindowNext(&(conn->sndwnd), sgm.hdr.plds);

		DBGPRINT(RUSP_DEBUG, "SND (NXT): base:%u nxt:%u end:%u SNDUSRBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));

		popStrBuff(&(conn->sndusrbuff), plds);
	}

	pthread_cleanup_pop(1);

	return NULL;
}

static void *receiverLoop(void *arg) {
	Connection *conn = (Connection *) arg;
	SgmBuffElem *curr = NULL;
	Segment rcvsgm;
	int rcvd;

	while (1) {

		rcvd = receiveSegment(conn, &rcvsgm);

		if (rcvd == 0) {
			if (getSgmBuffSize(&(conn->sndsgmbuff)) > 0)
				timeoutFunction(conn);
			continue;
		} else if (rcvd == -1) {
			setConnectionState(conn, RUSP_CLOSED);
			pthread_exit(NULL);
		}

		if (rcvsgm.hdr.ctrl & RUSP_SACK)
			submitSACK(conn, rcvsgm.hdr.ackn);
		else if (rcvsgm.hdr.ctrl & RUSP_CACK)
			submitCACK(conn, rcvsgm.hdr.ackn);

		switch (matchWindow(&(conn->rcvwnd), (rcvsgm.hdr.plds > 0)?rcvsgm.hdr.seqn + rcvsgm.hdr.plds - 1:rcvsgm.hdr.seqn)) {

		case 0:

			DBGPRINT(RUSP_DEBUG, "INSIDE RCVWND: base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));

			if (!((rcvsgm.hdr.ctrl == RUSP_SACK || rcvsgm.hdr.ctrl == RUSP_CACK) && rcvsgm.hdr.plds == 0))
				sendSACK(conn, RUSP_NXTSEQN(rcvsgm.hdr.seqn, (rcvsgm.hdr.ctrl & RUSP_FIN) ? 1 : rcvsgm.hdr.plds));

			if (rcvsgm.hdr.seqn == getWindowBase(&(conn->rcvwnd))) {

				DBGPRINT(RUSP_DEBUG, "IS RCVWNDB: %u", rcvsgm.hdr.seqn);

				processRcvWndBase(conn, rcvsgm);

				while ((curr = findSgmBuffSeqn(&(conn->rcvsgmbuff), getWindowBase(&(conn->rcvwnd))))) {

					Segment sgm = curr->segment;

					removeSgmBuff(&(conn->rcvsgmbuff), curr);

					processRcvWndBase(conn, sgm);
				}

			} else {

				if (!((rcvsgm.hdr.ctrl == RUSP_SACK || rcvsgm.hdr.ctrl == RUSP_CACK) && rcvsgm.hdr.plds == 0) && !findSgmBuffSeqn(&(conn->rcvsgmbuff), rcvsgm.hdr.seqn)) {
					DBGPRINT(RUSP_DEBUG, "BUFFERIZED: %u", rcvsgm.hdr.seqn);
					addSgmBuff(&(conn->rcvsgmbuff), rcvsgm, 0);
				} else {
					DBGPRINT(RUSP_DEBUG, "NOT BUFFERIZED: %u", rcvsgm.hdr.seqn);
				}

			}
			break;

		case -1:

			DBGPRINT(RUSP_DEBUG, "BEFORE RCVWND: base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));

			if (!((rcvsgm.hdr.ctrl == RUSP_SACK || rcvsgm.hdr.ctrl == RUSP_CACK) && rcvsgm.hdr.plds == 0))
				sendCACK(conn, getWindowBase(&(conn->rcvwnd)));
			break;

		default:
			DBGPRINT(RUSP_DEBUG, "OUTSIDE RCVWND: base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
			break;
		}
	}

	return NULL;
}

static void *timeWaitFunction(void *arg) {
	Connection *conn = (Connection *) arg;
	Segment rcvsgm;
	int rcvd;
	struct timespec start;

	start = getTimestamp();

	while (getElapsedNow(start) < RUSP_TIMEWTTM) {

		rcvd = receiveSegment(conn, &rcvsgm);

		if (rcvd == -1)
			break;

		if (rcvd == 0)
			continue;

		if ((matchWindow(&(conn->rcvwnd), rcvsgm.hdr.seqn) == 0) && (rcvsgm.hdr.ctrl & RUSP_FIN))
			sendSACK(conn, RUSP_NXTSEQN(rcvsgm.hdr.seqn, 1));
	}

	setConnectionState(conn, RUSP_CLOSED);

	destroyConnection(conn);

	return NULL;
}

/* SOCKET THREADS SUBPROCEDURES */

static void timeoutFunction(Connection *conn) {
	SgmBuffElem *curr = NULL;

	DBGPRINT(RUSP_DEBUG, "RETRANSMISSION");

	if (pthread_rwlock_rdlock(&(conn->sndsgmbuff.rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	curr = conn->sndsgmbuff.head;

	while (curr) {

		if (testSgmBuffElemAttributes(curr, RUSP_NACK, getTimeoutValue(&(conn->timeout)))) {

			updateSgmBuffElemAttributes(curr, 1, getTimeoutValue(&(conn->timeout)));

			sendSegment(conn, curr->segment);
		}

		curr = curr->next;
	}

	if (pthread_rwlock_unlock(&(conn->sndsgmbuff.rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	DBGPRINT(RUSP_DEBUG, "END OF RETRANSMISSION");
}

static void processRcvWndBase(Connection *conn, const Segment sgm) {

	switch (getConnectionState(conn)) {

		case RUSP_ESTABL:
			if (sgm.hdr.plds != 0) {
				writeStrBuff(&(conn->rcvusrbuff), sgm.pld, sgm.hdr.plds);
				slideWindow(&(conn->rcvwnd), sgm.hdr.plds);
				DBGPRINT(RUSP_DEBUG, "RCV (WND): base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
			}

			if (sgm.hdr.ctrl & RUSP_PSH)
				allignStrBuffSizeUsr(&(conn->rcvusrbuff));

			if (sgm.hdr.ctrl & RUSP_FIN) {
				allignStrBuffSizeUsr(&(conn->rcvusrbuff));
				setConnectionState(conn, RUSP_CLOSWT);
				slideWindow(&(conn->rcvwnd), 1);
				DBGPRINT(RUSP_DEBUG, "RCV (WND): base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
			}
			break;

		case RUSP_LSTACK:
			if ((sgm.hdr.ctrl & RUSP_SACK) && (sgm.hdr.ackn == getWindowNext(&(conn->sndwnd)))) {
				setConnectionState(conn, RUSP_CLOSED);
				pthread_exit(NULL);
			}
			break;

		case RUSP_FINWT1:
			if ((sgm.hdr.ctrl & RUSP_SACK) && (sgm.hdr.ackn == getWindowNext(&(conn->sndwnd)))) {
				setConnectionState(conn, RUSP_FINWT2);
			} else if ((sgm.hdr.ctrl & (RUSP_FIN | RUSP_SACK)) && (sgm.hdr.ackn != getWindowNext(&(conn->sndwnd)))) {
				setConnectionState(conn, RUSP_CLOSIN);
				slideWindow(&(conn->rcvwnd), 1);
				DBGPRINT(RUSP_DEBUG, "RCV (WND): base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
			} else if ((sgm.hdr.ctrl & (RUSP_FIN | RUSP_SACK)) && (sgm.hdr.ackn == getWindowNext(&(conn->sndwnd)))) {
				setConnectionState(conn, RUSP_TIMEWT);
				slideWindow(&(conn->rcvwnd), 1);
				DBGPRINT(RUSP_DEBUG, "RCV (WND): base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
				createThread(timeWaitFunction, conn, THREAD_DETACHED);
				pthread_exit(NULL);
			}
			break;

		case RUSP_FINWT2:
			if (sgm.hdr.ctrl & RUSP_FIN) {
				setConnectionState(conn, RUSP_TIMEWT);
				slideWindow(&(conn->rcvwnd), 1);
				DBGPRINT(RUSP_DEBUG, "RCV (WND): base:%u end:%u RCVUSRBUFF:%zu RCVSGMBUFF:%ld", getWindowBase(&(conn->rcvwnd)), getWindowEnd(&(conn->rcvwnd)), getStrBuffSize(&(conn->rcvusrbuff)), getSgmBuffSize(&(conn->rcvsgmbuff)));
				createThread(timeWaitFunction, conn, THREAD_DETACHED);
				pthread_exit(NULL);
			}
			break;

		case RUSP_CLOSIN:
			if ((sgm.hdr.ctrl & RUSP_SACK) && (sgm.hdr.ackn == getWindowNext(&(conn->sndwnd)))) {
				setConnectionState(conn, RUSP_TIMEWT);
				createThread(timeWaitFunction, conn, THREAD_DETACHED);
				pthread_exit(NULL);
			}
			break;

		default:
			break;
	}
}

static void submitSACK(Connection *conn, const uint32_t ackn) {
	SgmBuffElem *ackedelem = NULL;
	Segment sgm;
	long double sampleRTT;

	if ((ackedelem = findSgmBuffAckn(&(conn->sndsgmbuff), ackn))) {

		setSgmBuffElemStatus(ackedelem, RUSP_YACK);

		sgm = ackedelem->segment;

		DBGPRINT(RUSP_DEBUG, "SACKED: %u", sgm.hdr.seqn);

		if (sgm.hdr.seqn == getWindowBase(&(conn->sndwnd))) {

			sampleRTT = getSgmBuffElemElapsed(ackedelem);

			while (conn->sndsgmbuff.head) {

				if (getSgmBuffElemStatus(conn->sndsgmbuff.head) != RUSP_YACK)
					break;

				sgm = conn->sndsgmbuff.head->segment;

				removeSgmBuff(&(conn->sndsgmbuff), conn->sndsgmbuff.head);

				slideWindow(&(conn->sndwnd), (sgm.hdr.ctrl & RUSP_FIN)?1:sgm.hdr.plds);

				DBGPRINT(RUSP_DEBUG, "SND (WND): base:%u nxt:%u end:%u SNDUSRBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));
			}

			updateTimeout(&(conn->timeout), sampleRTT);
		}
	}
}

static void submitCACK(Connection *conn, const uint32_t ackn) {

	while (conn->sndsgmbuff.head) {

		Segment sgm = conn->sndsgmbuff.head->segment;

		if (!RUSP_LTSEQN(sgm.hdr.seqn, ackn))
			break;

		setSgmBuffElemStatus(conn->sndsgmbuff.head, RUSP_YACK);

		DBGPRINT(RUSP_DEBUG, "CACKED: %u", sgm.hdr.seqn);

		removeSgmBuff(&(conn->sndsgmbuff), conn->sndsgmbuff.head);

		slideWindow(&(conn->sndwnd), (sgm.hdr.ctrl & RUSP_FIN)?1:sgm.hdr.plds);

		DBGPRINT(RUSP_DEBUG, "SND (WND): base:%u nxt:%u end:%u SNDUSRBUFF:%zu SNDSGMBUFF:%ld", getWindowBase(&(conn->sndwnd)), getWindowNext(&(conn->sndwnd)), getWindowEnd(&(conn->sndwnd)), getStrBuffSize(&(conn->sndusrbuff)), getSgmBuffSize(&(conn->sndsgmbuff)));
	}
}

static void sendSACK(Connection *conn, const uint32_t ackn) {
	Segment acksgm;

	acksgm = createSegment(RUSP_SACK, 0, getWindowNext(&(conn->sndwnd)), ackn, NULL);

	sendSegment(conn, acksgm);
}

static void sendCACK(Connection *conn, const uint32_t ackn) {
	Segment acksgm;

	acksgm = createSegment(RUSP_CACK, 0, getWindowNext(&(conn->sndwnd)), ackn, NULL);

	sendSegment(conn, acksgm);
}

static void cleanupFunction(void *arg) {
	pthread_mutex_t *mtx = (pthread_mutex_t *) arg;

	pthread_mutex_trylock(mtx);

	pthread_mutex_unlock(mtx);
}

/* SEGMENT I/O */

static int sendSegment(Connection *conn, Segment sgm) {
	char ssgm[RUSP_SGMS + 1];
	size_t ssgmsize;

	if (!(sgm.hdr.ctrl & RUSP_SACK) & !(sgm.hdr.ctrl & RUSP_CACK)) {

		sgm.hdr.ctrl |= RUSP_SACK;

		sgm.hdr.ackn = getWindowBase(&(conn->rcvwnd));
	}

	ssgmsize = serializeSegment(sgm, ssgm);

	if (pthread_mutex_lock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot lock sock mutex for write.");

	if (writeCSocket(conn->sock.fd, ssgm, ssgmsize) == -1) {
		DBGPRINT(RUSP_DEBUG, "Cannot write connected socket: peer disconnected.");
		if (pthread_mutex_unlock(&(conn->sock.mtx)) > 0)
			ERREXIT("Cannot unlock mutex.");
		return -1;
	}

	if (pthread_mutex_unlock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");

	DBGFUNC(RUSP_DEBUG, printOutSegment(getSocketPeer(conn->sock.fd), sgm));

	return 1;
}

static int receiveSegment(Connection *conn, Segment *sgm) {
	char ssgm[RUSP_SGMS];
	long double timeout;

	timeout = getTimeoutValue(&(conn->timeout));

	if (selectSocket(conn->sock.fd, timeout) == 0)
		return 0;

	if (pthread_mutex_lock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot lock sock mutex for read.");

	if (readCSocket(conn->sock.fd, ssgm, RUSP_SGMS) == -1) {
		if (pthread_mutex_unlock(&(conn->sock.mtx)) > 0)
			ERREXIT("Cannot unlock sock mutex for read.");
		DBGPRINT(RUSP_DEBUG, "Cannot read connected socket: peer disconnected.");
		return -1;
	}

	if (pthread_mutex_unlock(&(conn->sock.mtx)) > 0)
		ERREXIT("Cannot unlock sock mutex for read.");

	deserializeSegment(ssgm, sgm);

	if (getRandomBit(RUSP_DROP)) {
		char strsgm[RUSP_SGM_STR+1];
		segmentToString(*sgm, strsgm);
		DBGPRINT(RUSP_DEBUG, "SEGMENT DROPPPED: %s", strsgm);

		return 0;
	}

	DBGFUNC(RUSP_DEBUG, printInSegment(getSocketPeer(conn->sock.fd), *sgm));

	return 1;
}

/* CONNECTIONS POOL */

Connection *getConnectionById(const ConnectionId connid) {
	Connection *conn = NULL;

	conn = (Connection *) getElementById(&CONPOOL, connid);

	return conn;		
}
