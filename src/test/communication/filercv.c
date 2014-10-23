#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "../../rudp.h"
#include "../../util/fileutil.h"

static int PORT;

static char *FILERCV;

static char *FILEMTX;

static int DEBUGMODE;

static ConnectionId lconn;

static ConnectionId aconn;

static void startListen(void);

static void showListeningConnectionDetails(void);

static void acceptIncomingConnection(void);

static void stopListen(void);

static void showEstablishedConnectionDetails(void);

static void fileReceive(void);

int main(int argc, char **argv) {

	if (argc < 3)
		ERREXIT("usage: %s [port] [filercv] [filemtx] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	FILERCV = argv[2];

	FILEMTX = argv[3];

	DEBUGMODE = atoi(argv[4]);

	setConnectionDebugMode(DEBUGMODE);

	startListen();

	showListeningConnectionDetails();

	acceptIncomingConnection();	

	stopListen();

	showEstablishedConnectionDetails();	

	fileReceive();	

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	printf("# Opening listening connection on port: %d\n", PORT);

	lconn = rudpListen(PORT);

	if (lconn == -1) 
		ERREXIT("Cannot setup listening connection.");
}

static void showListeningConnectionDetails(void) {
	struct sockaddr_in laddr;
	char strladdr[ADDRIPV4_STR];

	laddr = rudpGetLocalAddress(lconn);

	addressToString(laddr, strladdr);

	printf("Connection (%ld) listening on: %s.\n", lconn, strladdr);		
}

static void acceptIncomingConnection(void) {
	printf("# Accepting incoming connection\n");

	aconn = rudpAccept(lconn);

	printf("SUCCESS\n");
}

static void stopListen(void) {
	printf("# Closing listening connection\n");

	rudpClose(lconn);

	printf("SUCCESS\n");
}

static void showEstablishedConnectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char straaddr[ADDRIPV4_STR], strcaddr[ADDRIPV4_STR];

	aaddr = rudpGetLocalAddress(aconn);

	addressToString(aaddr, straaddr);

	caddr = rudpGetPeerAddress(aconn);

	addressToString(caddr, strcaddr);

	printf("Connection (%ld) established on: %s with: %s.\n", aconn, straaddr, strcaddr);		
}

static void fileReceive(void) {
	char rcvdata[500];
	size_t rcvd;
	int fdrcv, fdmtx;

	fdrcv = openFile(FILERCV, O_RDWR|O_CREAT|O_TRUNC);

	printf("# Receiving file on established connection...\n");

	while ((rcvd = rudpReceive(aconn, rcvdata, 500)) > 0) {

		if (rcvd == 1 && rcvdata[0] == 0x01) {
			printf("Received: EOF\n");
			break;
		}

		printf("Received (%zu): %.*s\n", rcvd, (int) rcvd, rcvdata);

		errno = 0;
		if (write(fdrcv, rcvdata, rcvd) == -1)
			ERREXIT("Cannot write to file: %s.", strerror(errno));

		memset(rcvdata, 0, 500);
	}

	fdmtx = openFile(FILEMTX, O_RDONLY);

	assert(isEqualFile(fdrcv, fdmtx));

	closeFile(fdrcv);

	closeFile(fdmtx);

	printf("OK\n");
}
