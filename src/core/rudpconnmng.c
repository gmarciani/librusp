#include "rudpconnmng.h"

static int DEBUG = 0;

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

			DBGFUNC(DEBUG, printOutSegment(laddr, syn));

			setConnectionState(conn, RUDP_CON_SYNS);

			ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_SGMS);

			if (ssynack == NULL) {
				DBGPRINT(DEBUG, "SYNACK DROPPED");
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
				DBGPRINT(DEBUG, "SYN DROPPED");
				continue;
			}				
			
			syn = deserializeSegment(ssyn);

			DBGFUNC(DEBUG, printInSegment(caddr, syn));

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

			DBGFUNC(DEBUG, printOutSegment(caddr, synack));

			setConnectionState(lconn, RUDP_CON_SYNS);	

			sacksynack = readUnconnectedSocket(asock, &caddr, RUDP_SGMS);

			if (sacksynack == NULL) {
				DBGPRINT(DEBUG, "ACK SYNACK DROPPED");
				continue;
			}

			clock_gettime(CLOCK_MONOTONIC, &end);

			sampleRTT = getElapsed(start, end);

			acksynack = deserializeSegment(sacksynack);	

			DBGFUNC(DEBUG, printInSegment(caddr, acksynack));

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

/* MESSAGE I/O */

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
