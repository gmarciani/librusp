#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../rusp.h"
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
	int DBGON = 1;
	int DBGOFF = 0;

	if (argc < 3)
		ERREXIT("usage: %s [port] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	DBG = atoi(argv[2]);

	if (DBG & DBG_OPEN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	startListen();

	listenDetails();

	acceptConnection();	

	stopListen();

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	connectionDetails();

	if (DBG & DBG_TRAN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	echo();	

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	if (DBG & DBG_CLOS)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	closeConnection();

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Opening listening connection on port: %d...%s", PORT, dbg?"\n":"");

	LCONN = ruspListen(PORT);

	if (LCONN == -1) 
		ERREXIT("Cannot setup listening connection.");

	printf("OK\n");
}

static void listenDetails(void) {
	struct sockaddr_in laddr;
	char strladdr[ADDRIPV4_STR];

	ruspLocal(LCONN, &laddr);

	addressToString(laddr, strladdr);

	printf("Connection (%lld) listening on: %s.\n", LCONN, strladdr);
}

static void acceptConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Accepting incoming connection...%s", dbg?"\n":"");

	CONN = ruspAccept(LCONN);

	printf("OK\n");
}

static void stopListen(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing listening connection...%s", dbg?"\n":"");

	ruspClose(LCONN);

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char straaddr[ADDRIPV4_STR], strcaddr[ADDRIPV4_STR];

	ruspLocal(CONN, &aaddr);

	addressToString(aaddr, straaddr);

	ruspPeer(CONN, &caddr);

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

		rcvd = ruspReceive(CONN, rcvdata, MSGS);

		if (rcvd <= 0)
			break;

		printf("[RCV]>%.*s\n", (int) rcvd, rcvdata);

		assert(rcvd == MSGS);

		assert(strncmp(rcvdata, MSG, MSGS) == 0);

		ruspSend(CONN, rcvdata, rcvd);

		printf("[SND]>%.*s\n", (int) rcvd, rcvdata);

		memset(rcvdata, 0, sizeof(char) * MSGS);

		iteration++;
	}

	printf("Stop receiving on established connection...OK\n");
}

static void closeConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing established connection...%s", dbg?"\n":"");

	ruspClose(CONN);

	printf("OK\n");
}
