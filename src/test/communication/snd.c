#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/timeutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static char *ADDRESS;

static int PORT;

static double DROP;

static int DBG;

static ConnectionId CONN;

static void establishConnection(void);

static void connectionDetails(void);

static void sendInput(void);

static void closeConnection(void);

int main(int argc, char **argv) {	
	
	if (argc < 5)
		ERREXIT("usage: %s [address] [port] [drop] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROP = strtod(argv[3], NULL);

	DBG = atoi(argv[4]);

	rudpSetDrop(DROP);

	if (DBG & DBG_OPEN)
		rudpSetDebug(1);

	establishConnection();

	rudpSetDebug(0);

	connectionDetails();

	if (DBG & DBG_TRAN)
		rudpSetDebug(1);

	sendInput();

	rudpSetDebug(0);

	if (DBG & DBG_CLOS)
		rudpSetDebug(1);

	closeConnection();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, (rudpGetDebug())?"\n":"");

	CONN = rudpConnect(ADDRESS, PORT);

	if (CONN == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	rudpGetLocalAddress(CONN, &caddr);

	addressToString(caddr, strcaddr);
	
	rudpGetPeerAddress(CONN, &saddr);

	addressToString(saddr, strsaddr);

	printf("# Connection (%lld) established on: %s with: %s.\n", CONN, strcaddr, strsaddr);
}

static void sendInput(void) {
	char *input = NULL;
	ssize_t snd;

	printf("# Sending on established connection (drop %F%%)...\n", rudpGetDrop() * 100.0);

	while (1) {

		input = getUserInput("[INPUT (empty to disconnect)]>");

		if ((snd = strlen(input)) == 0) {
			free(input);
			break;
		}

		rudpSend(CONN, input, snd);

		free(input);
	}

	printf("# Stop sending on established connection...OK\n");
}

static void closeConnection(void) {
	printf("# Closing established connection...%s", (rudpGetDebug())?"\n":"");

	rudpClose(CONN);

	printf("OK\n");
}
