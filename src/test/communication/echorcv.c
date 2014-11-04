#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../rudp.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

#define MSG "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnooooopppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzz"

#define MSGS strlen(MSG)

static int PORT;

static int DBG;

static ConnectionId LCONN;

static ConnectionId CONN;

static void startListen(void);

static void listenDetails(void);

static void acceptConnection(void);

static void stopListen(void);

static void connectionDetails(void);

static void echo(void);

static void closeConnection(void);

int main(int argc, char **argv) {

	if (argc < 3)
		ERREXIT("usage: %s [port] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	DBG = atoi(argv[2]);

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

	echo();	

	rudpSetDebug(0);

	if (DBG & DBG_CLOS)
		rudpSetDebug(1);

	closeConnection();

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
	printf("# Closing listening connection...%s", (rudpGetDebug())?"\n":"");

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

static void echo(void) {
	char rcvdata[MSGS];
	ssize_t rcvd;
	long long iteration;

	printf("# Echo on established connection...\n\n");

	iteration = 1;

	while (1) {

		printf("\n%lld\n", iteration);

		rcvd = rudpReceive(CONN, rcvdata, MSGS);

		if (rcvd <= 0)
			break;

		printf("[RCV]>%.*s\n", (int) rcvd, rcvdata);

		assert(rcvd == MSGS);

		assert(strncmp(rcvdata, MSG, MSGS) == 0);

		rudpSend(CONN, rcvdata, rcvd);

		printf("[SND]>%.*s\n", (int) rcvd, rcvdata);

		memset(rcvdata, 0, sizeof(char) * MSGS);

		iteration++;
	}

	printf("Stop receiving on established connection...OK\n");
}

static void closeConnection(void) {
	printf("# Closing established connection...%s", (rudpGetDebug())?"\n":"");

	rudpClose(CONN);

	printf("OK\n");
}
