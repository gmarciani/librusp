#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "rusp.h"

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
	int DBGON = 1;
	int DBGOFF = 0;
	
	if (argc < 5)
		ERREXIT("usage: %s [address] [port] [drop] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROP = strtod(argv[3], NULL);

	DBG = atoi(argv[4]);

	ruspSetAttr(RUSP_ATTR_DROPR, &DROP);

	if (DBG & DBG_OPEN)
		ruspSetAttr(RUSP_ATTR_DEBUG, &DBGON);

	establishConnection();

	ruspSetAttr(RUSP_ATTR_DEBUG, &DBGOFF);

	connectionDetails();

	if (DBG & DBG_TRAN)
		ruspSetAttr(RUSP_ATTR_DEBUG, &DBGON);

	sendInput();

	ruspSetAttr(RUSP_ATTR_DEBUG, &DBGOFF);

	if (DBG & DBG_CLOS)
		ruspSetAttr(RUSP_ATTR_DEBUG, &DBGON);

	closeConnection();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, dbg?"\n":"");

	CONN = ruspConnect(ADDRESS, PORT);

	if (CONN == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	ruspLocal(CONN, &caddr);

	addressToString(caddr, strcaddr);
	
	ruspPeer(CONN, &saddr);

	addressToString(saddr, strsaddr);

	printf("# Connection (%lld) established on: %s with: %s.\n", CONN, strcaddr, strsaddr);
}

static void sendInput(void) {
	char *input = NULL;
	ssize_t snd;
	double drop;

	ruspGetAttr(RUSP_ATTR_DROPR, &drop);

	printf("# Sending on established connection (drop %F%%)...\n", drop * 100.0);

	while (1) {

		input = getUserInput("[INPUT (empty to disconnect)]>");

		if ((snd = strlen(input)) == 0) {
			free(input);
			break;
		}

		ruspSend(CONN, input, snd);

		free(input);
	}

	printf("# Stop sending on established connection...OK\n");
}

static void closeConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing established connection...%s", dbg?"\n":"");

	ruspClose(CONN);

	printf("OK\n");
}
