#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "../../rudp.h"
#include "../../util/fileutil.h"

#define BUFFSIZE 1024

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

	FILERCV = stringDuplication(argv[2]);

	FILEMTX = stringDuplication(argv[3]);

	DEBUGMODE = atoi(argv[4]);

	setConnectionDebugMode(DEBUGMODE);

	startListen();

	showListeningConnectionDetails();

	acceptIncomingConnection();	

	stopListen();

	showEstablishedConnectionDetails();	

	fileReceive();	

	free(FILERCV);

	free(FILEMTX);

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
	char *strladdr = NULL;

	laddr = rudpGetLocalAddress(lconn);

	strladdr = addressToString(laddr);	

	printf("Connection (%ld) listening on: %s.\n", lconn, strladdr);		

	free(strladdr);
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
	char *straaddr, *strcaddr = NULL;

	aaddr = rudpGetLocalAddress(aconn);

	straaddr = addressToString(aaddr);

	caddr = rudpGetPeerAddress(aconn);

	strcaddr = addressToString(caddr);	

	printf("Connection (%ld) established on: %s with: %s.\n", aconn, straaddr, strcaddr);		

	free(straaddr);	

	free(strcaddr);
}

static void fileReceive(void) {
	char *rcvdata = NULL;
	size_t rcvd;
	int fdrcv, fdmtx;

	fdrcv = openFile(FILERCV, O_RDWR|O_CREAT|O_TRUNC);

	printf("# Receiving file on established connection...");

	while (1) {

		rcvdata = rudpReceive(aconn, BUFFSIZE);

		if (!rcvdata)
			break;

		errno = 0;
		if (write(fdrcv, rcvdata, strlen(rcvdata)) != rcvd)
			ERREXIT("Cannot write to file: %s.", strerror(errno));

		free(rcvdata);
	}

	fdmtx = openFile(FILEMTX, O_RDONLY);

	assert(isEqualFile(fdrcv, fdmtx));

	closeFile(fdrcv);

	closeFile(fdmtx);

	printf("OK\n");
}
