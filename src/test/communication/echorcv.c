#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../rudp.h"
#include "../../util/cliutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define MSG "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnooooopppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzz"

#define MSGSIZE strlen(MSG)

static int PORT;

static int DEBUGMODE;

static ConnectionId lconn;

static ConnectionId aconn;

static void startListen(void);

static void showListeningConnectionDetails(void);

static void acceptIncomingConnection(void);

static void stopListen(void);

static void showEstablishedConnectionDetails(void);

static void echo(void);

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

	echo();	

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

static void echo(void) {
	char rcvdata[MSGSIZE];
	ssize_t rcvd;
	long long iteration;

	printf("# Echo on established connection...\n\n");

	iteration = 1;

	while (1) {

		printf("\n%lld\n", iteration);

		rcvd = rudpReceive(aconn, rcvdata, MSGSIZE);

		if (rcvd <= 0)
			break;

		printf("[RCV]>%.*s\n", (int) rcvd, rcvdata);

		//progressCounter(iteration);

		assert(rcvd == MSGSIZE);

		assert(strncmp(rcvdata, MSG, MSGSIZE) == 0);

		rudpSend(aconn, rcvdata, rcvd);

		printf("[SND]>%.*s\n", (int) rcvd, rcvdata);

		memset(rcvdata, 0, sizeof(char) * MSGSIZE);

		iteration++;
	}

	printf("Stop receiving on established connection...OK\n");

	printf("# Closing established connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(aconn);

	printf("OK\n");
}
