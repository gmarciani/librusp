#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../util/fileutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static int PORT;

static char *FILERCV;

static char *FILEMTX;

static int LCONN;

static int CONN;

static void startListen(void);

static void listenDetails(void);

static void acceptConnection(void);

static void stopListen(void);

static void connectionDetails(void);

static void receiveFile(void);

static void closeConnection(void);

static void matchReceivedFile(void);

int main(int argc, char **argv) {

	if (argc < 4)
		ERREXIT("usage: %s [port] [filercv] [filemtx]", argv[0]);

	PORT = atoi(argv[1]);

	FILERCV = argv[2];

	FILEMTX = argv[3];

	startListen();

	listenDetails();

	acceptConnection();	

	stopListen();

	connectionDetails();

	receiveFile();

	closeConnection();

	matchReceivedFile();

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	struct sockaddr_in addr;

	printf("# Opening listening connection on port: %d...", PORT);

	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;

	addr.sin_port = htons(PORT);

	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((LCONN = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		ERREXIT("Cannot setup listening connection.");

	if (bind(LCONN, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
		ERREXIT("Cannot setup listening connection.");

	if (listen(LCONN, SOMAXCONN) == -1)
		ERREXIT("Cannot setup listening connection.");

	printf("OK\n");
}

static void listenDetails(void) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in laddr;
	char ip[INET_ADDRSTRLEN];
	int port;

	errno = 0;
	if (getsockname(LCONN, (struct sockaddr *)&laddr, &socksize) == -1)
		ERREXIT("Cannot get socket local address: %s", strerror(errno));

	if (!inet_ntop(AF_INET, &(laddr.sin_addr), ip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	port = (int) ntohs(laddr.sin_port);

	printf("Connection listening on: %s:%d.\n", ip, port);
}

static void acceptConnection(void) {
	socklen_t socksize = sizeof(struct sockaddr_in);

	printf("# Accepting incoming connection...");

	if ((CONN = accept(LCONN, NULL, &socksize)) == -1)
		ERREXIT("Cannot accept incoming connections.");

	printf("OK\n");
}

static void stopListen(void) {
	printf("# Closing listening connection...");

	if (close(LCONN) == -1)
		ERREXIT("Cannot close listening connection.");

	printf("OK\n");
}

static void connectionDetails(void) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in aaddr, caddr;
	char aip[INET_ADDRSTRLEN], cip[INET_ADDRSTRLEN];
	int aport, cport;

	errno = 0;
	if (getsockname(CONN, (struct sockaddr *)&aaddr, &socksize) == -1)
		ERREXIT("Cannot get socket local address: %s", strerror(errno));

	if (!inet_ntop(AF_INET, &(aaddr.sin_addr), aip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	aport = (int) ntohs(aaddr.sin_port);

	errno = 0;
	if (getpeername(CONN, (struct sockaddr *)&caddr, &socksize) == -1)
		ERREXIT("Cannot get socket peer address: %s", strerror(errno));

	if (!inet_ntop(AF_INET, &(caddr.sin_addr), cip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	cport = (int) ntohs(caddr.sin_port);

	printf("Connection established on: %s:%d with: %s:%d.\n", aip, aport, cip, cport);
}

static void receiveFile(void) {
	char rcvdata[500];
	ssize_t rcvd;
	int fdrcv;

	fdrcv = openFile(FILERCV, O_RDWR|O_CREAT|O_TRUNC);

	printf("# Receiving file on established connection...");

	while ((rcvd = recv(CONN, rcvdata, 500, 0)) > 0) {

		errno = 0;
		if (write(fdrcv, rcvdata, rcvd) == -1)
			ERREXIT("Cannot write to file: %s.", strerror(errno));

		memset(rcvdata, 0, sizeof(char) * 500);
	}

	printf("OK\n");

	printf("Stop receiving on established connection...OK\n");

	closeFile(fdrcv);
}

static void closeConnection(void) {
	printf("# Closing established connection...");

	if (close(CONN) == -1)
		ERREXIT("Cannot close established connection.");

	printf("OK\n");
}

static void matchReceivedFile(void) {
	int fdrcv, fdmtx;

	printf("# Matching received file (%s) with matrix file (%s)...", FILERCV, FILEMTX);

	fdrcv = openFile(FILERCV, O_RDONLY);

	fdmtx = openFile(FILEMTX, O_RDONLY);

	assert(isEqualFile(fdrcv, fdmtx));

	closeFile(fdrcv);

	closeFile(fdmtx);

	printf("OK\n");
}
