#include "rudp.h"

static int PKT_RESOLUTION = 0;


/* CONNECTION */

int rudpListen(const int lport) {
	int lsock;
	struct sockaddr_in laddr;

	lsock = _rudpOpenSocket();

	laddr = _rudpAddress("127.0.0.1", lport);

	_rudpReusableSocket(lsock);

	_rudpBindSocket(lsock, &laddr);

	return lsock;
}

rudpConnection_t rudpConnect(const struct sockaddr_in saddr) {
	rudpConnection_t conn;
	struct sockaddr_in asaddr;
	int sock;
	packet_t pktSYN;
	packet_t pktSYNACK;
	packet_t pktACKSYNACK;

	sock = _rudpOpenSocket();

	_rudpCreatePacket(SYN, 0, 0, NULL, &pktSYN);

	__rudpSendPacket(sock, saddr, pktSYN);

	do {
		__rudpReceivePacket(sock, &asaddr, &pktSYNACK);
	} while (pktSYNACK.header.control != ACK);

	_rudpConnectSocket(sock, asaddr);

	conn.sockfd = sock;
	conn.local  = _rudpSocketLocal(conn.sockfd);
	conn.peer   = _rudpSocketPeer(conn.sockfd);

	_rudpCreatePacket(ACK, 0, 0, NULL, &pktACKSYNACK);

	_rudpSendPacket(conn, pktACKSYNACK);

	return conn;
}

rudpConnection_t rudpAccept(const int lsock) {
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

void rudpDisconnect(rudpConnection_t *conn, int mode) {
	switch (mode) {
		case DCONN_SOFT:
			_rudpCloseSocket(conn->sockfd);
			break;
		case DCONN_HARD:
			_rudpCloseSocket(conn->sockfd);
			break;
	}
}

void rudpCloseSocket(const int sockfd) {
	_rudpCloseSocket(sockfd);
}


/* COMMUNICATION */

void rudpSend(const rudpConnection_t conn, const char *message) {
	packet_t *packets = NULL;
	int numPackets;
	int i;	

	packets = _rudpPacketStream(message, &numPackets);

	for (i = 0; i < numPackets; i++) {
		_rudpSendPacket(conn, packets[i]);
		if (PKT_RESOLUTION)
			_rudpPrintOutPacket(conn.peer, packets[i]);
	}

	free(packets);
}

char *rudpReceive(const rudpConnection_t conn) {
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

		if (PKT_RESOLUTION)
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


/* ADDRESS */

struct sockaddr_in rudpAddress(const char *ip, const int port) {
	return _rudpAddress(ip, port);
}

int rudpIsEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo) {
	return _rudpIsEqualAddress(addrOne, addrTwo);
}

struct sockaddr_in rudpSocketAddress(const int sockfd) {
	return _rudpSocketLocal(sockfd);
}

char *rudpGetAddress(const struct sockaddr_in addr) {
	return _rudpGetAddress(addr);
}

int rudpGetPort(const struct sockaddr_in addr) {
	return _rudpGetPort(addr);
}


/* SETTING */

void rudpPacketResolution(const int resolution) {
	PKT_RESOLUTION = resolution;
}
