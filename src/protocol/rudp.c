#include "rudp.h"

static int RUDP_DEBUG = 1;

/* CONNECTION */

int rudpListen(const int lport) {
	struct sockaddr_in laddr;
	int lsock;	

	laddr = _rudpAddress("127.0.0.1", lport);

	lsock = _rudpOpenSocket();	

	_rudpReusableSocket(lsock);

	_rudpBindSocket(lsock, &laddr);

	return lsock;
}

rudpconn_t rudpConnect(const char *ip, const int port) {
	struct sockaddr_in lsaddr, asaddr;
	rudpconn_t conn;
	rudpsgm_t sgmSYN, sgmSYNACK, sgmACKSYNACK;

	lsaddr = _rudpAddress(ip, port);

	conn.sock = _rudpOpenSocket();

	sgmSYN _rudpCreateSegment(_RUDP_VERSION, unsigned short int hdrs, unsigned short int ctrl, unsigned short int plds, const char *seqno, const char *ackno, const char *wndno, const char *pld);

	__rudpSendPacket(conn.sock, lsaddr, pktSYN);

	do {
		__rudpReceivePacket(sock, &asaddr, &pktSYNACK);
	} while (pktSYNACK.header.control != ACK);

	_rudpConnectSocket(conn.sock, asaddr);

	conn.sockfd = sock;
	conn.local  = _rudpSocketLocal(conn.sockfd);
	conn.peer   = _rudpSocketPeer(conn.sockfd);

	pktACKSYNACK = _rudpCreatePacket(ACK, 0, 0, NULL);

	_rudpSendPacket(conn, pktACKSYNACK);

	return conn;
}

void rudpAccept(rudpConn_t *conn) {
	rudpConnection_t conn;
	int sock;
	struct sockaddr_in caddr;
	struct sockaddr_in tmpcaddr;
	packet_t pktSYN;
	packet_t pktSYNACK;
	packet_t pktACKSYNACK;

	do {
		__rudpReceivePacket(lsock, &caddr, &pktSYN);
	} while (pktSYN.header.control != SYN);

	sock = _rudpOpenSocket();	

	_rudpCreatePacket(ACK, 0, 0, NULL, &pktSYNACK);

	__rudpSendPacket(sock, caddr, pktSYNACK);

	do {
		__rudpReceivePacket(sock, &tmpcaddr, &pktACKSYNACK);
	} while ((!_rudpIsEqualAddress(tmpcaddr, caddr)) | (pktACKSYNACK.header.control != ACK));	

	_rudpConnectSocket(sock, caddr);

	conn.sockfd  = sock;
	conn.local   = _rudpSocketLocal(conn.sockfd);
	conn.peer    = _rudpSocketPeer(conn.sockfd); 

	return conn;
}

void rudpDisconnect(rudpConn_t *conn) {
	switch (mode) {
		case DCONN_SOFT:
			_rudpCloseSocket(conn->sockfd);
			break;
		case DCONN_HARD:
			_rudpCloseSocket(conn->sockfd);
			break;
	}
}

/* COMMUNICATION */

void rudpSend(const rudpConn_t *conn, const char *msg) {
	packet_t *packets = NULL;
	int numPackets;
	int i;	

	packets = _rudpPacketStream(msg, &numPackets);

	for (i = 0; i < numPackets; i++) {
		_rudpSendPacket(conn, packets[i]);
		if (RUDP_DEBUG)
			_rudpPrintOutPacket(conn.peer, packets[i]);
	}

	free(packets);
}

char *rudpReceive(const rudpConn_t *conn, const size_t rcvSize) {
	packet_t packet;
	char *message = NULL;
	char *buffer = NULL;
	size_t bufferSize;
	size_t read;

	bufferSize = PACKET_SERIALIZATION_SIZE + 1; 

	if (!(buffer = malloc(sizeof(char) * bufferSize))) {
		fprintf(stderr, "Error in buffer allocation for packet receive.\n");
		exit(EXIT_FAILURE);
	}

	buffer[0] = '\0';	

	read = 0;
	do {
		_rudpReceivePacket(conn, &packet);

		if (DEBUG)
			_rudpPrintInPacket(conn.peer, packet);

		buffer = strcat(buffer, packet.payload);

		read += strlen(packet.payload);

		buffer[read] = '\0';	

		if (read + PACKET_SERIALIZATION_SIZE + 1 >= bufferSize) {
			bufferSize *= 2;	

			if (!(buffer = realloc(buffer, bufferSize))) {
				fprintf(stderr, "Error in buffer reallocation for packet receive.\n");
				exit(EXIT_FAILURE);
			}
		}			

	} while (!packet.header.streamEnd);

	message = stringDuplication(buffer);

	free(buffer);

	return message;
}
