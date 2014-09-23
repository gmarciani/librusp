#include "_rudp.h"


/* SETTINGS */

static int DTG_RESOLUTION = 0;


/* PACKET */

void _rudpPacketDeserialization(const char *serializedPacket, packet_t *packet) {
	char **args = NULL;
	int expectedArgs = 4;
	int i;

	args = splitStringNByDelimiter(serializedPacket, PACKET_FIELDS_DELIMITER, expectedArgs);

	_rudpParsePacket(args[0], args[1], args[2], args[3], packet);	

	for (i = 0; i < expectedArgs; i++)
		free(args[i]);

	free(args);
}

char *_rudpPacketSerialization(const packet_t packet) {
	char *serializedPacket = NULL;

	if (!(serializedPacket = malloc(sizeof(char) * (PACKET_SERIALIZATION_SIZE + 1)))) {
		fprintf(stderr, "Error in serialized packet allocation.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(serializedPacket, "%d%s%d%s%d%s%s", packet.header.control, PACKET_FIELDS_DELIMITER, packet.header.sequenceNumber, PACKET_FIELDS_DELIMITER, packet.header.streamEnd, PACKET_FIELDS_DELIMITER, packet.payload);

	return serializedPacket;
}

void _rudpParsePacket(const char *control, const char *sequenceNumber, const char *streamEnd, const char *payload, packet_t *packet) {
	int pktControl;
	int pktSequenceNumber;
	int pktStreamEnd;

	pktControl = atoi(control);
	pktSequenceNumber = atoi(sequenceNumber);
	pktStreamEnd = atoi(streamEnd);

	_rudpCreatePacket(pktControl, pktSequenceNumber, pktStreamEnd, payload, packet);
}

void _rudpCreatePacket(const int control, const int sequenceNumber, const int streamEnd, const char *payload, packet_t *packet) {
	size_t payloadSize = 0;
	int i;

	packet->header.control = control;
	packet->header.sequenceNumber = sequenceNumber;	
	packet->header.streamEnd = streamEnd;

	if (payload) {
		payloadSize = (strlen(payload) < PACKET_PAYLOAD_SIZE) ? strlen(payload) : PACKET_PAYLOAD_SIZE;
		for (i = 0; i < payloadSize; i++)
			packet->payload[i] = payload[i];
	}

	packet->payload[payloadSize] = '\0';
	
}

packet_t *_rudpPacketStream(const char *message, int *numPackets) {
	packet_t *packets = NULL;
	char **chunks = NULL;
	size_t chunkSize;
	int sequenceNumber;
	int i, j;

	chunks = splitStringBySize(message, PACKET_PAYLOAD_SIZE, numPackets);

	if (!(packets = malloc(sizeof(packet_t) * *numPackets))) {
		fprintf(stderr, "Error in packet stream allocation: %s.\n", message);
		exit(EXIT_FAILURE);
	}

	sequenceNumber = 0;
	for (i = 0; i < *numPackets; i++) {
		packets[i].header.control = DAT;
		chunkSize = strlen(chunks[i]);		
		packets[i].header.sequenceNumber = sequenceNumber;
		packets[i].header.streamEnd = 0;
		for (j = 0; j < chunkSize; j++) {
			packets[i].payload[j] = chunks[i][j];
		}			
		packets[i].payload[chunkSize] = '\0';
		sequenceNumber += chunkSize;
	}

	packets[*numPackets - 1].header.streamEnd = 1;

	for (i = 0; i < *numPackets; i++)
		free(chunks[i]);
	
	free(chunks);

	return packets;
}


/* SOCKET */

int _rudpOpenSocket() {
	int sockfd;

	errno = 0;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Error in socket creation: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

void _rudpCloseSocket(const int sockfd) {
	errno = 0;
	if (close(sockfd) == -1) {
		fprintf(stderr, "Error in closing socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpBindSocket(const int sockfd, struct sockaddr_in *addr) {
	errno = 0;
	if (bind(sockfd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in socket binding: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpConnectSocket(const int sockfd, const struct sockaddr_in addr) {
	errno = 0;
	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in connecting socket: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpReusableSocket(const int sockfd) {
	int optval = 1;

	errno = 0;
  	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1) {
		fprintf(stderr, "Error in setsockopt: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}


/* COMMUNICATION */

void _rudpSendPacket(const rudpConnection_t conn, const packet_t packet) {
	char *serializedPacket = NULL;

	serializedPacket = _rudpPacketSerialization(packet);

	if (write(conn.sockfd, serializedPacket, PACKET_SERIALIZATION_SIZE) == -1) {
		fprintf(stderr, "Error in socket write.\n");
		exit(EXIT_FAILURE);
	}

	if (DTG_RESOLUTION)
		_rudpPrintOutDatagram(conn.peer, serializedPacket);

	free(serializedPacket);
}

void _rudpReceivePacket(const rudpConnection_t conn, packet_t *packet) {	
	char *serializedPacket = NULL;
	ssize_t received;

	if (!(serializedPacket = malloc(sizeof(char) * (PACKET_SERIALIZATION_SIZE + 1)))) {
		fprintf(stderr, "Error in serialized packet allocation for packet receive.\n");
		exit(EXIT_FAILURE);
	}

	received = 0;
	if ((received = read(conn.sockfd, serializedPacket, PACKET_SERIALIZATION_SIZE)) == -1) {
		fprintf(stderr, "Error in socket read.\n");
		exit(EXIT_FAILURE);
	}

	serializedPacket[received] = '\0';

	if (DTG_RESOLUTION)
		_rudpPrintInDatagram(conn.peer, serializedPacket);

	_rudpPacketDeserialization(serializedPacket, packet);

	free(serializedPacket);
}

void __rudpSendPacket(const int sockfd, const struct sockaddr_in dest, const packet_t packet) {
	char *serializedPacket = NULL;

	serializedPacket = _rudpPacketSerialization(packet);

	if (sendto(sockfd, serializedPacket, PACKET_SERIALIZATION_SIZE, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in packet send: %s.\n", serializedPacket);
		exit(EXIT_FAILURE);
	}

	if (DTG_RESOLUTION)
		_rudpPrintOutDatagram(dest, serializedPacket);

	free(serializedPacket);
}

void __rudpReceivePacket(const int sockfd, struct sockaddr_in *source, packet_t *packet) {
	socklen_t socksize = sizeof(struct sockaddr_in);	
	char *serializedPacket = NULL;
	ssize_t received;

	if (!(serializedPacket = malloc(sizeof(char) * (PACKET_SERIALIZATION_SIZE + 1)))) {
		fprintf(stderr, "Error in serialized packet allocation for packet receive.\n");
		exit(EXIT_FAILURE);
	}

	received = 0;
	if ((received = recvfrom(sockfd, serializedPacket, PACKET_SERIALIZATION_SIZE, 0 ,(struct sockaddr *)source, &socksize)) == -1) {
		fprintf(stderr, "Error in packet receive.\n");
		exit(EXIT_FAILURE);
	}

	serializedPacket[received] = '\0';

	if (DTG_RESOLUTION)
		_rudpPrintInDatagram(*source, serializedPacket);

	_rudpPacketDeserialization(serializedPacket, packet);

	free(serializedPacket);
}


/* ADDRESS */

struct sockaddr_in _rudpAddress(const char *ip, const int port) {
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (!ip) {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);	
	} else {
		if (inet_pton(AF_INET, ip, &(addr.sin_addr)) <= 0) {
			fprintf(stderr, "Error in address to-network translation: %s:%d.\n", ip, port);
			exit(EXIT_FAILURE);
		}
	}

	return addr;
}

int _rudpIsEqualAddress(const struct sockaddr_in addrOne, const struct sockaddr_in addrTwo) {
	return ((addrOne.sin_family == addrTwo.sin_family ) &&
			(addrOne.sin_port == addrTwo.sin_port) &&
			(addrOne.sin_addr.s_addr == addrTwo.sin_addr.s_addr));
}

struct sockaddr_in _rudpSocketLocal(const int sockfd) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getsockname(sockfd, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getsockname: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}

struct sockaddr_in _rudpSocketPeer(const int sockfd) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getpeername(sockfd, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getpeername: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}

char *_rudpGetAddress(const struct sockaddr_in address) {
	char *str = NULL;

	if (!(str = malloc(INET_ADDRSTRLEN))) {
		fprintf(stderr, "Error in serialized address allocation.\n");
		exit(EXIT_FAILURE);
	}

	if (!inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN)) {
		fprintf(stderr, "Error in address to-presentation translation.\n");
		exit(EXIT_FAILURE);
	}

	return str;
}

int _rudpGetPort(const struct sockaddr_in address) {
	return (int) ntohs(address.sin_port);
}


/* OUTPUT */

void _rudpPrintInPacket(const struct sockaddr_in sender, const packet_t packet) {
	char *time = NULL;
	char *address = NULL;
	int port;

	time = getTime();
	address = _rudpGetAddress(sender);
	port = _rudpGetPort(sender);

	printf("[<- PKT] (%d) %s src: %s:%d ctl: %d sqn: %d sen: %d pld: %s\n", getpid(), time, address, port, packet.header.control, packet.header.sequenceNumber, packet.header.streamEnd, packet.payload);

	free(time);
	free(address);
}

void _rudpPrintOutPacket(const struct sockaddr_in receiver, const packet_t packet) {
	char *time = NULL;
	char *address = NULL;
	int port;

	time = getTime();
	address = _rudpGetAddress(receiver);
	port = _rudpGetPort(receiver);
	
	printf("[PKT ->] (%d) %s dst: %s:%d ctl: %d sqn: %d sen: %d pld: %s\n", getpid(), time, address, port, packet.header.control, packet.header.sequenceNumber, packet.header.streamEnd, packet.payload);

	free(time);
	free(address);
}

void _rudpPrintInDatagram(const struct sockaddr_in sender, const char *datagram) {
	char *time = NULL;
	char *address = NULL;
	int port;

	time = getTime();
	address = _rudpGetAddress(sender);
	port = _rudpGetPort(sender);

	printf("[<- DTG] (%d) %s src: %s:%d pld: %s\n", getpid(), time, address, port, datagram);

	free(time);
	free(address);
}

void _rudpPrintOutDatagram(const struct sockaddr_in receiver, const char *datagram) {
	char *time = NULL;
	char *address = NULL;
	int port;

	time = getTime();
	address = _rudpGetAddress(receiver);
	port = _rudpGetPort(receiver);
	
	printf("[DTG ->] (%d) %s dst: %s:%d pld: %s\n", getpid(), time, address, port, datagram);

	free(time);
	free(address);
}


/* SETTING */

void _rudpDatagramResolution(const int resolution) {
	DTG_RESOLUTION = resolution;
}
