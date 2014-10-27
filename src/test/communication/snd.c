#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/timeutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

static char *ADDRESS;

static int PORT;

static double DROPRATE;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void sendInput(void);

int main(int argc, char **argv) {	
	
	if (argc < 5)
		ERREXIT("usage: %s [address] [port] [drop] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROPRATE = strtod(argv[3], NULL);

	DEBUGMODE = atoi(argv[4]);

	setDropRate(DROPRATE);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	sendInput();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, (DEBUGMODE)?"\n":"");

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void showConnectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	caddr = rudpGetLocalAddress(conn);

	addressToString(caddr, strcaddr);
	
	saddr = rudpGetPeerAddress(conn);

	addressToString(saddr, strsaddr);

	printf("# Connection (%lld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);
}

static void sendInput(void) {
	char *input = NULL;
	ssize_t snd;

	printf("# Sending on established connection (drop %F%%)...\n", DROPRATE * 100.0);

	while (1) {

		input = getUserInput("[INPUT (empty to disconnect)]>");

		if ((snd = strlen(input)) == 0) {
			free(input);
			break;
		}

		rudpSend(conn, input, snd);

		free(input);
	}

	printf("# Stop sending on established connection...OK\n");

	printf("# Closing established connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(conn);

	printf("OK\n");
}
