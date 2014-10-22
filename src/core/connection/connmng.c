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
	Segment syn, synack, acksynack;
	char ssyn[RUDP_SGMS + 1], ssynack[RUDP_SGMS + 1], sacksynack[RUDP_SGMS + 1];
	int asock, synretrans;
	struct sockaddr_in aaddr;
	struct timespec start, end;
	long double sampleRTT;	

	if (getConnectionState(conn) != RUDP_CON_CLOS)
		ERREXIT("Cannot synchronize connection: connection not closed.");

	asock = openSocket();

	syn = createSegment(RUDP_SYN, 0, 0, 0, 0, 0, NULL);

	serializeSegment(syn, ssyn);

	for (synretrans = 0; synretrans < RUDP_SYN_RETR; synretrans++) {

		clock_gettime(CLOCK_MONOTONIC, &start);

		writeUSocket(asock, laddr, ssyn, strlen(ssyn));

		DBGFUNC(DEBUG, printOutSegment(laddr, syn));

		setConnectionState(conn, RUDP_CON_SYNS);

		if (!selectSocket(asock, RUDP_SAMPLRTT))
			continue;

		readUSocket(asock, &aaddr, ssynack, RUDP_SGMS);

		clock_gettime(CLOCK_MONOTONIC, &end);

		sampleRTT = getElapsed(start, end);

		deserializeSegment(ssynack, &synack);

		DBGFUNC(DEBUG, printInSegment(aaddr, synack));

		if ((synack.hdr.ctrl == (RUDP_SYN | RUDP_ACK)) &
			(synack.hdr.ackn == RUDP_NXTSEQN(syn.hdr.seqn, 1))) {

			setConnectionState(conn, RUDP_CON_SYNR);

			acksynack = createSegment(RUDP_ACK, 0, 0, 0, RUDP_NXTSEQN(syn.hdr.seqn, 1), RUDP_NXTSEQN(synack.hdr.seqn, 1), NULL);

			serializeSegment(acksynack, sacksynack);

			writeUSocket(asock, aaddr, sacksynack, strlen(sacksynack));

			DBGFUNC(DEBUG, printOutSegment(aaddr, acksynack));

			setupConnection(conn, asock, aaddr, acksynack.hdr.seqn, acksynack.hdr.ackn, sampleRTT);

			setConnectionState(conn, RUDP_CON_ESTA);

			return conn->connid;
		}
	}

	closeSocket(asock);

	setConnectionState(conn, RUDP_CON_CLOS);

	return -1;
}

ConnectionId acceptSynchonization(Connection *lconn) {
	Connection *aconn = NULL;
	Segment syn, synack, acksynack;
	char ssyn[RUDP_SGMS + 1], ssynack[RUDP_SGMS + 1], sacksynack[RUDP_SGMS + 1];
	int asock, synackretrans;
	struct sockaddr_in caddr;
	struct timespec start, end;
	long double sampleRTT;

	while (getConnectionState(lconn) == RUDP_CON_LIST) {

		readUSocket(lconn->sock.fd, &caddr, ssyn, RUDP_SGMS);

		deserializeSegment(ssyn, &syn);

		DBGFUNC(DEBUG, printInSegment(caddr, syn));

		if (syn.hdr.ctrl != RUDP_SYN)
			continue;

		setConnectionState(lconn, RUDP_CON_SYNR);

		asock = openSocket();
	
		synack = createSegment(RUDP_SYN | RUDP_ACK, 0, 0, 0, 10, RUDP_NXTSEQN(syn.hdr.seqn, 1), NULL);

		serializeSegment(synack, ssynack);

		for (synackretrans = 0; synackretrans < RUDP_CON_RETR; synackretrans++) {

			clock_gettime(CLOCK_MONOTONIC, &start);

			writeUSocket(asock, caddr, ssynack, strlen(ssynack));

			DBGFUNC(DEBUG, printOutSegment(caddr, synack));

			setConnectionState(lconn, RUDP_CON_SYNS);	

			if (!selectSocket(asock, RUDP_SAMPLRTT))
				continue;

			readUSocket(asock, &caddr, sacksynack, RUDP_SGMS);

			clock_gettime(CLOCK_MONOTONIC, &end);

			sampleRTT = getElapsed(start, end);

			deserializeSegment(sacksynack, &acksynack);

			DBGFUNC(DEBUG, printInSegment(caddr, acksynack));

			if ((acksynack.hdr.ctrl == RUDP_ACK) &
				(acksynack.hdr.seqn == synack.hdr.ackn) &
				(acksynack.hdr.ackn == RUDP_NXTSEQN(synack.hdr.seqn, 1))) {
	
				aconn = createConnection();	

				setConnectionState(aconn, RUDP_CON_SYNS);

				setupConnection(aconn, asock, caddr, acksynack.hdr.ackn, acksynack.hdr.seqn, sampleRTT);

				setConnectionState(aconn, RUDP_CON_ESTA);

				return aconn->connid;			
			}
		}

		closeSocket(asock);

		setConnectionState(lconn, RUDP_CON_LIST);	
	}

	return -1;
}

/* DESYNCHRONIZATION */

void desynchronizeConnection(Connection *conn) {
	destroyConnection(conn);
}
