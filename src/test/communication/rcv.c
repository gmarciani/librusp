#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../rudp.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

static int PORT;

static int DEBUGMODE;

static ConnectionId lconn;

static ConnectionId aconn;

static void startListen(void);

static void showListeningConnectionDetails(void);

static void acceptIncomingConnection(void);

static void stopListen(void);

static void showEstablishedConnectionDetails(void);

static void receiveInput(void);

int main(int argc, char **argv) {

	if (argc < 3)
		ERREXIT("usage: %s [port] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	DEBUGMODE = atoi(argv[2]);

	setConnectionDebugMode(DEBUGMODE);

	startListen();

	showListeningConnectionDetails();

	acceptIncomingConnection();	

	stopListen();

	showEstablishedConnectionDetails();	

	receiveInput();

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	printf("# Opening listening connection on port: %d...%s", PORT, (DEBUGMODE)?"\n":"");

	lconn = rudpListen(PORT);

	if (lconn == -1) 
		ERREXIT("Cannot setup listening connection.");

	printf("OK\n");
}

static void showListeningConnectionDetails(void) {
	struct sockaddr_in laddr;
	char strladdr[ADDRIPV4_STR];

	laddr = rudpGetLocalAddress(lconn);

	addressToString(laddr, strladdr);

	printf("Connection (%lld) listening on: %s.\n", lconn, strladdr);
}

static void acceptIncomingConnection(void) {
	printf("# Accepting incoming connection...%s", (DEBUGMODE)?"\n":"");

	aconn = rudpAccept(lconn);

	printf("OK\n");
}

static void stopListen(void) {
	printf("# Closing listening connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(lconn);

	printf("OK\n");
}

static void showEstablishedConnectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char straaddr[ADDRIPV4_STR], strcaddr[ADDRIPV4_STR];

	aaddr = rudpGetLocalAddress(aconn);

	addressToString(aaddr, straaddr);

	caddr = rudpGetPeerAddress(aconn);

	addressToString(caddr, strcaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", aconn, straaddr, strcaddr);
}

static void receiveInput(void) {
	char rcvdata[500];
	ssize_t rcvd;

	printf("# Receiving on established connection...\n");

	while ((rcvd = rudpReceive(aconn, rcvdata, 500)) > 0) {

		printf("[RCVD] %.*s\n", (int) rcvd, rcvdata);
		
		memset(rcvdata, 0, sizeof(char) * 500);
	}

	printf("Stop receiving on established connection...OK\n");

	printf("# Closing established connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(aconn);

	printf("OK\n");
}
