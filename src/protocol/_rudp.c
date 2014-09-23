#include "_rudp.h"

static int _RUDP_DEBUG = 1;

/* PACKET */

rudpsgm_t _rudpDeserializeSegment(const char *ssgm) {
	rudpsgm_t sgm;
	size_t pldsize;
	char **fields;
	int efields, i;

	efields = 7;

	fields = splitStringNByDelimiter(spkt, _RUDP_SGM_DEL, efields);

	sgm.hdr.vers = (unsigned short int) atoi(fiels[0]);
	sgm.hdr.ctrl = (unsigned short int) atoi(fiels[1]);
	sgm.hdr.plds = (unsigned short int) atoi(fiels[2]);
	sgm.hdr.sqnno = strtoul(fiels[3], NULL, 10);
	sgm.hdr.ackno = strtoul(fiels[4], NULL, 10);
	sgm.hdr.wndno = strtoul(fiels[5], NULL, 10);

	pldsize = (sgm.hdr.plds < _RUDP_MAX_PLD) ? sgm.hdr.plds : _RUDP_MAX_PLD;

	for (i = 0; i < pldsize; i++)
		sgm.pld[i] = fields[6][i];

	sgm.pld[i] = '\0';	

	for (i = 0; i < efields; i++)
		free(fields[i]);
	free(fields);

	return sgm;
}

char *_rudpSerializeSegment(const rudpsgm_t sgm) {
	char *ssgm;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(ssgm, "%02d%s%d%s%d%s%d%s%d%s%d%s%d%s%s", sgm.hdr.vers, _RUDP_SGM_DEL, sgm.hdr.hdrs, _RUDP_SGM_DEL, sgm.hdr.ctrl, _RUDP_SGM_DEL, sgm.hdr.plds, _RUDP_SGM_DEL, sgm.hdr.seqno, _RUDP_SGM_DEL, sgm.hdr.ackno, _RUDP_SGM_DEL, sgm.hdr.wndno, _RUDP_SGM_DEL, sgm.pld);

	return ssgm;
}

rudpsgm_t _rudpCreateSegment(const unsigned short int ctrl, unsigned long int seqno, unsigned long int ackno, unsigned long int wndno, const char *pld) {
	rudpsgm_t sgm;
	int i = 0;

	sgm.vers = _RUDP_VERSION;
	sgm.ctrl = ctrl;
	sgm.sqnno = sqnno;
	sgm.ackno = ackno;
	sgm.wndno = wndno;

	if (pld) {
		pldsize = (strlen(pld) < _RUDP_MAX_PLD) ? strlen(pld) : _RUDP_MAX_PLD;
		for (i = 0; i < pldsize; i++)
			sgm.pld[i] = pld[i];
	}

	sgm.pld[i] = '\0';

	return sgm;	
}

/* COMMUNICATION */

void _rudpSendSegment(const rudpconn_t conn, const rudpsgm_t sgm) {
	char *ssgm;

	ssgm = _rudpSerializeSegment(sgm);

	if (write(conn.sock, ssgm, _RUDP_MAX_SGM) == -1) {
		fprintf(stderr, "Error in socket write.\n");
		exit(EXIT_FAILURE);
	}

	if (_RUDP_DEBUG)
		_rudpPrintOutDatagram(conn.peer, ssgm);

	free(ssgm);
}

rudpsgm_t _rudpReceiveSegment(const rudpconn_t conn) {	
	rudpsgm_t sgm;
	char *ssgm;
	ssize_t rcvd;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation for packet receive.\n");
		exit(EXIT_FAILURE);
	}

	rcvd = 0;
	if ((rcvd = read(conn.sock, ssgm, _RUDP_MAX_SGM)) == -1) {
		fprintf(stderr, "Error in socket read.\n");
		exit(EXIT_FAILURE);
	}

	ssgm[rcvd] = '\0';

	if (_RUDP_DEBUG)
		_rudpPrintInDatagram(conn.peer, ssgm);

	sgm = _rudpDeserializeSegment(ssgm);

	free(ssgm);

	return sgm;
}

void __rudpSendSegment(const int sock, const struct sockaddr_in rcvaddr, const rudpsgm_t sgm) {
	char *ssgm;

	ssgm = _rudpSerializeSegment(sgm);

	if (sendto(sock, ssgm, _RUDP_MAX_SGM, 0, (struct sockaddr *)&rcvaddr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in segment send: %s.\n", ssgm);
		exit(EXIT_FAILURE);
	}

	if (_RUDP_DEBUG)
		_rudpPrintOutDatagram(rcvaddr, ssgm);

	free(ssgm);
}

rudpsgm_t __rudpReceiveSegment(const int sock, struct sockaddr_in *sndaddr) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	rudpsgm_t sgm;	
	char *ssgm;
	ssize_t rcvd;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation.\n");
		exit(EXIT_FAILURE);
	}

	rcvd = 0;
	if ((rcvd = recvfrom(sock, ssgm, _RUDP_MAX_SGM, 0 ,(struct sockaddr *)sndaddr, &socksize)) == -1) {
		fprintf(stderr, "Error in segment receive.\n");
		exit(EXIT_FAILURE);
	}

	ssgm[rcvd] = '\0';

	if (_RUDP_DEBUG)
		_rudpPrintInDatagram(*sndaddr, ssgm);

	smg = _rudpDeserializeSegment(ssgm);

	free(ssgm);

	return sgm;
}

/* SOCKET */

int _rudpOpenSocket() {
	int sock;

	errno = 0;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Error in socket creation: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return sock;
}

void _rudpCloseSocket(const int sock) {
	errno = 0;
	if (close(sock) == -1) {
		fprintf(stderr, "Error in closing socket: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpBindSocket(const int sock, struct sockaddr_in *addr) {
	errno = 0;
	if (bind(sock, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in socket binding: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpConnectSocket(const int sock, const struct sockaddr_in addr) {
	errno = 0;
	if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Error in connecting socket: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void _rudpReusableSocket(const int sock) {
	int optval = 1;

	errno = 0;
  	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) == -1) {
		fprintf(stderr, "Error in setsockopt: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
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

struct sockaddr_in _rudpSocketLocal(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getsockname(sock, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getsockname: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}

struct sockaddr_in _rudpSocketPeer(const int sock) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	errno = 0;
	if (getpeername(sock, (struct sockaddr *)&addr, &socksize) == -1) {
		fprintf(stderr, "Error in getpeername: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return addr;
}

char *_rudpGetAddress(const struct sockaddr_in addr) {
	char *str;

	if (!(str = malloc(INET_ADDRSTRLEN))) {
		fprintf(stderr, "Error in serialized address allocation.\n");
		exit(EXIT_FAILURE);
	}

	if (!inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN)) {
		fprintf(stderr, "Error in address to-presentation translation.\n");
		exit(EXIT_FAILURE);
	}

	return str;
}

int _rudpGetPort(const struct sockaddr_in address) {
	return (int) ntohs(address.sin_port);
}

/* OUTPUT */

void _rudpPrintInSegment(const struct sockaddr_in sndaddr, const rudpsgm_t sgm) {
	char *time, *addr;
	int port;

	time = getTime();
	addr = _rudpGetAddress(sndaddr);
	port = _rudpGetPort(sndaddr);

	printf("[<- SGM] (%d) %s src: %s:%d vers:%hu hdrs:%hu ctrl:%hu plds:%hu seqno:%lu ackno:%lu wndno:%lu pld:%s\n", getpid(), time, addr, port, sgm.hdr.vers, sgm.hdr.hdrs, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.hdr.wndno, sgm.pld);

	free(time);
	free(addr);
}

void _rudpPrintOutSegment(const struct sockaddr_in rcvaddr, const rudpsgm_t sgm) {
	char *time, *addr;
	int port;

	time = getTime();
	addr = _rudpGetAddress(rcvaddr);
	port = _rudpGetPort(rcvaddr);
	
	printf("[SGM ->] (%d) %s dst: %s:%d vers:%hu hdrs:%hu ctrl:%hu plds:%hu seqno:%lu ackno:%lu wndno:%lu pld:%s\n", getpid(), time, addr, port, sgm.hdr.vers, sgm.hdr.hdrs, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.hdr.wndno, sgm.pld);

	free(time);
	free(addr);
}

void _rudpPrintInDatagram(const struct sockaddr_in sndaddr, const char *dtg) {
	char *time, *addr;
	int port;

	time = getTime();
	addr = _rudpGetAddress(sndaddr);
	port = _rudpGetPort(sndaddr);

	printf("[<- DTG] (%d) %s src: %s:%d pld: %s\n", getpid(), time, addr, port, dtg);

	free(time);
	free(addr);
}

void _rudpPrintOutDatagram(const struct sockaddr_in rcvaddr, const char *dtg) {
	char *time, *addr;
	int port;

	time = getTime();
	addr = _rudpGetAddress(rcvaddr);
	port = _rudpGetPort(rcvaddr);
	
	printf("[DTG ->] (%d) %s dst: %s:%d pld: %s\n", getpid(), time, addr, port, dtg);

	free(time);
	free(addr);
}
