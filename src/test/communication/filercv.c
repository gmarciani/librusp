#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "../../rudp.h"
#include "../../util/fileutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static int PORT;

static char *FILERCV;

static char *FILEMTX;

static int DBG;

static ConnectionId LCONN;

static ConnectionId CONN;

static void startListen(void);

static void listenDetails(void);

static void acceptConnection(void);

static void stopListen(void);

static void connectionDetails(void);

static void receiveFile(void);

static void closeConnection(void);

static void matchReceivedFile(void);

int main(int argc, char **argv) {

	if (argc < 3)
		ERREXIT("usage: %s [port] [filercv] [filemtx] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	FILERCV = argv[2];

	FILEMTX = argv[3];

	DBG = atoi(argv[4]);

	if (DBG & DBG_OPEN)
		rudpSetDebug(1);

	startListen();

	listenDetails();

	acceptConnection();	

	stopListen();

	rudpSetDebug(0);

	connectionDetails();

	if (DBG & DBG_TRAN)
		rudpSetDebug(1);

	receiveFile();

	rudpSetDebug(0);

	if (DBG & DBG_CLOS)
		rudpSetDebug(1);

	closeConnection();

	matchReceivedFile();

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	printf("# Opening listening connection on port: %d...%s", PORT, (rudpGetDebug())?"\n":"");

	LCONN = rudpListen(PORT);

	if (LCONN == -1) 
		ERREXIT("Cannot setup listening connection.");

	printf("OK\n");
}

static void listenDetails(void) {
	struct sockaddr_in laddr;
	char strladdr[ADDRIPV4_STR];

	rudpGetLocalAddress(LCONN, &laddr);

	addressToString(laddr, strladdr);

	printf("Connection (%lld) listening on: %s.\n", LCONN, strladdr);
}

static void acceptConnection(void) {
	printf("# Accepting incoming connection...%s", (rudpGetDebug())?"\n":"");

	CONN = rudpAccept(LCONN);

	printf("OK\n");
}

static void stopListen(void) {
	printf("# Closing listening connection...%s", (DBG)?"\n":"");

	rudpClose(LCONN);

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char straaddr[ADDRIPV4_STR], strcaddr[ADDRIPV4_STR];

	rudpGetLocalAddress(CONN, &aaddr);

	addressToString(aaddr, straaddr);

	rudpGetPeerAddress(CONN, &caddr);

	addressToString(caddr, strcaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", CONN, straaddr, strcaddr);
}

static void receiveFile(void) {
	char rcvdata[500];
	ssize_t rcvd;
	int fdrcv;

	fdrcv = openFile(FILERCV, O_RDWR|O_CREAT|O_TRUNC);

	printf("# Receiving file on established connection...");

	while ((rcvd = rudpReceive(CONN, rcvdata, 500)) > 0) {

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
	printf("# Closing established connection...%s", (rudpGetDebug())?"\n":"");

	rudpClose(CONN);

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
