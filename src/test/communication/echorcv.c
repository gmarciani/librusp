#include <stdlib.h>
#include <stdio.h>
#include "../../rudp.h"
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

static void echo(void) {
	char *rcvdata = NULL;
	unsigned long iteration;

	printf("# Echoing on established connection\n");

	iteration = 1;

	while (1) {
		
		printf("\nECHO %lu\n", iteration);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n", rcvdata);

		free(rcvdata);

		iteration++;
	}
}
