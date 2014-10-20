#include "connmng.h"

static int DEBUG = 0;

/* LISTENING */

void setListeningConnection(Connection *conn, const struct sockaddr_in laddr) {
	
	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot setup listening connection: connection not closed.");

	conn->sock.fd = openSocket();

	setSocketReusable(conn->sock.fd);
	
	bindSocket(conn->sock.fd, &laddr);

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
	int asock, synretrans;
	struct timespec start, end;
	long double sampleRTT;	

	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	syn = createSegment(RUDP_SYN, 0, 0, 0, 0, NULL);

	ssyn = serializeSegment(syn);

	for (synretrans = 0; synretrans < RUDP_SYN_RETR; synretrans++) {

		clock_gettime(CLOCK_MONOTONIC, &start);

		writeUnconnectedSocket(asock, laddr, ssyn);

		DBGFUNC(DEBUG, printOutSegment(laddr, syn));

		setConnectionState(conn, RUDP_CON_SYNS);

		if (!selectSocket(asock, RUDP_SAMPLRTT))
			continue;

		ssynack = readUnconnectedSocket(asock, &aaddr, RUDP_SGMS);

		clock_gettime(CLOCK_MONOTONIC, &end);

		sampleRTT = getElapsed(start, end);

		synack = deserializeSegment(ssynack);

		free(ssynack);

		DBGFUNC(DEBUG, printInSegment(aaddr, synack));

		if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &
			(synack.hdr.ackn == RUDP_NXTSEQN(syn.hdr.seqn, 1))) {

			setConnectionState(conn, RUDP_CON_SYNR);

			acksynack = createSegment(RUDP_ACK, 0, 0, RUDP_NXTSEQN(syn.hdr.seqn, 1), RUDP_NXTSEQN(synack.hdr.seqn, 1), NULL);

			sacksynack = serializeSegment(acksynack);

			writeUnconnectedSocket(asock, aaddr, sacksynack);

			DBGFUNC(DEBUG, printOutSegment(aaddr, acksynack));

			setupConnection(conn, asock, aaddr, acksynack.hdr.seqn, acksynack.hdr.ackn, sampleRTT);

			setConnectionState(conn, RUDP_CON_ESTA);

			free(ssyn);

			free(sacksynack);

			return conn->connid;
		}
	}

	free(ssyn);

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
	int asock, synackretrans;
	struct timespec start, end;
	long double sampleRTT;

	while (getConnectionState(lconn) == RUDP_CON_LIST) {

		ssyn = readUnconnectedSocket(lconn->sock.fd, &caddr, RUDP_SGMS);

		syn = deserializeSegment(ssyn);

		DBGFUNC(DEBUG, printInSegment(caddr, syn));

		free(ssyn);

		if (syn.hdr.ctrl !=RUDP_SYN)
			continue;

		setConnectionState(lconn, RUDP_CON_SYNR);

		asock = openSocket();
	
		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, 10, RUDP_NXTSEQN(syn.hdr.seqn, 1), NULL); 

		ssynack = serializeSegment(synack);

		for (synackretrans = 0; synackretrans < RUDP_CON_RETR; synackretrans++) {

			clock_gettime(CLOCK_MONOTONIC, &start);

			writeUnconnectedSocket(asock, caddr, ssynack);	

			DBGFUNC(DEBUG, printOutSegment(caddr, synack));

			setConnectionState(lconn, RUDP_CON_SYNS);	

			if (!selectSocket(asock, RUDP_SAMPLRTT))
				continue;

			sacksynack = readUnconnectedSocket(asock, &caddr, RUDP_SGMS);

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
		}

		free(ssynack);				

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CON_LIST);	
	}

	return -1;
}

/* DESYNCHRONIZATION */

void desynchronizeConnection(Connection *conn) {
	destroyConnection(conn);
}
