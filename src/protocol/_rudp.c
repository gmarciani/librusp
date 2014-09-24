#include "_rudp.h"

static int _RUDP_DEBUG = 1;

/* CONNECTION */

rudpconn_t _rudpConnInit() {
	rudpconnt_t conn;

	conn.version = _RUDP_VERSION;
	conn.status = _RUDP_CONN_CLOSED;
	conn.sock = _rudpOpenSocket();	
	conn.seqno = _rudpGetISN();
	conn.ackno = 0;
	conn.wndn = _RUDP_WND;

	return conn;
}

rudpconn_t _rudpASYNHandshake(const struct sockaddr_in laddr) {
	rudpconn_t conn;
	struct sockaddr_in aaddr;
	rudpsgm_t syn, synack, acksynack;

	conn = _rudpConnInit();

	syn = _rudpCreateSegment(_RUDP_SYN, conn.lseqno, conn.lackno, conn.lwndno, NULL);

	__rudpSendSegment(conn.sock, laddr, syn);

	_rudpSocketTimeout(_RUP_ACK_TIMEOUT);

	conn.lseqno = (conn.lseqno + 1) % ULONG_MAX;	

	do {
		synack = __rudpReceiveSegment(conn.sock, &aaddr);
	while ((synack.hdr.ctrl != _RDUP_ACK) | (synack.ackno != conn.lseqno))

	conn.status = _RUDP_CONN_SYN_SENT;

	_rudpConnectSocket(conn.sock, aaddr);
	conn.laddr = _rudpSocketLocal(conn.sock);
	conn.paddr = _rudpSocketPeer(conn.sock);
	conn.lackno = synack.hdr.seqno + 1;
	conn.pwndno = synack.hdr.wndno;		

	acksynack = _rudpCreateSegment(_RUDP_ACK, conn.lseqno, conn.lackno, conn.lwndno, NULL);
	
	_rudpSendSegment(&conn, acksynack);

	conn.status = _RUDP_CONN_ESTABLISHED;

	return conn;
}

rudpconn_t _rudpPSYNHandshake(const int lsock) {
	rudpconn_t conn;
	struct sockaddr_in caddr;
	rudpsgm_t syn, synack, acksynack;

	conn = _rudpConnInit();

	do {
		syn = __rudpReceiveSegment(lsock, &caddr);
	} while (syn.hdr.ctrl != _RUDP_SYN);

	conn.lackno = (syn.hdr.seqno + 1) % ULONG_MAX;
	conn.pwndno = syn.hdr.wndno;

	conn.status = _RUDP_CONN_SYN_RCVD;

	synack = _rudpCreateSegment(_RUDP_ACK, conn.lseqno, conn.lackno, conn.lwndno, NULL);

	__rudpSendSegment(conn.sock, caddr, synack);

	do {
		acksynack = __
	} while (acksynack.hdr.ctrl != ACK) | );

	conn.status = _RUDP_CONN_ESTABLISHED;

	return conn;
}

void _rudpFINHanshake(rudpconn_t *conn) {

}

unsigned long int _rudpGetISN() {
	unsigned long int isn;
	struct timeval time;

	gettimeofday(&time, NULL);
	isn = (1000000 * time.tv_sec + time.tv_usec) % 4;

	return isn;
}

/* SEGMENT */

rudpsgm_t _rudpDeserializeSegment(const char *ssgm) {
	rudpsgm_t sgm;
	char *hdr, **hdrfields;
	size_t hdrfieldssize[_RUDP_HDR_FIELDS] = {2, 2, 5, 10, 10, 10};
	size_t pldsize;
	int i;

	hdr = stringNDuplication(ssgm, _RUDP_MAX_HDR);

	hdrfields = splitStringBySection(hdr, hdrfieldssize, _RUDP_HDR_FIELDS);

	sgm.hdr.vers = (unsigned short int) atoi(hdrfields[0]);
	sgm.hdr.ctrl = (unsigned short int) atoi(hdrfields[1]);
	sgm.hdr.plds = (unsigned short int) atoi(hdrfields[2]);
	sgm.hdr.seqno = strtoul(hdrfields[3], NULL, 10);
	sgm.hdr.ackno = strtoul(hdrfields[4], NULL, 10);
	sgm.hdr.wndno = strtoul(hdrfields[5], NULL, 10);

	pldsize = (sgm.hdr.plds < _RUDP_MAX_PLD) ? sgm.hdr.plds : _RUDP_MAX_PLD;

	for (i = 0; i < pldsize; i++)
		sgm.pld[i] = *(ssgm + _RUDP_MAX_HDR + i);

	sgm.pld[i] = '\0';	

	for (i = 0; i < _RUDP_HDR_FIELDS; i++)
		free(hdrfields[i]);
	free(hdrfields);

	return sgm;
}

char *_rudpSerializeSegment(const rudpsgm_t sgm) {
	char *ssgm;

	if (!(ssgm = malloc(sizeof(char) * (_RUDP_MAX_SGM + 1)))) {
		fprintf(stderr, "Error in serialized segment allocation.\n");
		exit(EXIT_FAILURE);
	}

	sprintf(ssgm, "%02hu%02hu%05hu%010lu%010lu%010lu%s", sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.hdr.wndno,  sgm.pld);

	return ssgm;
}

rudpsgm_t _rudpCreateSegment(const unsigned short int ctrl, unsigned long int seqno, unsigned long int ackno, unsigned long int wndno, const char *pld) {
	rudpsgm_t sgm;
	size_t pldsize = 0;
	int i;

	sgm.hdr.vers = _RUDP_VERSION;
	sgm.hdr.ctrl = ctrl;
	sgm.hdr.seqno = seqno;
	sgm.hdr.ackno = ackno;
	sgm.hdr.wndno = wndno;

	if (pld) {
		pldsize = (strlen(pld) < _RUDP_MAX_PLD) ? strlen(pld) : _RUDP_MAX_PLD;
		for (i = 0; i < pldsize; i++)
			sgm.pld[i] = pld[i];
	}

	sgm.pld[pldsize] = '\0';

	sgm.hdr.plds = pldsize;

	return sgm;	
}

/* STREAM */

rupdstream_t _rudpSegmentStream(const char *msg) {
	rudpstream_t stream;
	char *chunks;
	size_t chunksize;
	int numchunks, i, j;

	chunks = splitStringBySize(msg, _RUDP_MAX_PLD, &numchunks);

	if (stream.segments = malloc(sizeof(rudpsgm_t) * numchunks)) {
		fprintf(stderr, "Error in segment stream allocation.\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < numchunks; i++) {
		stream.segments[i].version = _RUDP_VERSION;
		stream.segments[i].ctrl = _RUDP_DAT;
		chunksize = strlen(chunks[i]);
		for (j = 0; j < chunksize; j++) {
			stream.segments[i].plds++;
			stream.segments[i][j] = chunks[i][j];
			stream.streamsize++;
		}
		stream.numsegments++;
	}

	for (i = 0; i < numchunks; i++) 
		free(chunks[i]);
	free(chunks);

	return stream;
}

void _rudpSendStream(const rudpstream_t stream) {
	
}

rudpstream_t _rudpReceiveStream(rudpconn_t *conn) {

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

void _rudpTimeoutSocket(const int sock, const long int timeout) {
	struct timeval timer;

	timer.tv_sec = 0;
  	timer.tv_usec = timeout;

	errno = 0;  
  	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&timer,sizeof(timer)) < 0) {
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

	printf("[<- SGM] (%d) %s src: %s:%d vers:%hu ctrl:%hu plds:%hu seqno:%lu ackno:%lu wndno:%lu pld:%s\n", getpid(), time, addr, port, sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.hdr.wndno, sgm.pld);

	free(time);
	free(addr);
}

void _rudpPrintOutSegment(const struct sockaddr_in rcvaddr, const rudpsgm_t sgm) {
	char *time, *addr;
	int port;

	time = getTime();
	addr = _rudpGetAddress(rcvaddr);
	port = _rudpGetPort(rcvaddr);
	
	printf("[SGM ->] (%d) %s dst: %s:%d vers:%hu ctrl:%hu plds:%hu seqno:%lu ackno:%lu wndno:%lu pld:%s\n", getpid(), time, addr, port, sgm.hdr.vers, sgm.hdr.ctrl, sgm.hdr.plds, sgm.hdr.seqno, sgm.hdr.ackno, sgm.hdr.wndno, sgm.pld);

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
