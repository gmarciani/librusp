#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/timeutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define MSG "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnooooopppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzz"

#define MSGSIZE strlen(MSG)

static char *ADDRESS;

static int PORT;

static long double DROPRATE;

static unsigned long ITERATIONS;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileEcho();

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [iterations] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROPRATE = strtold(argv[3], NULL);

	ITERATIONS = strtoul(argv[4], NULL, 10);

	DEBUGMODE = atoi(argv[5]);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	profileEcho();

	//disconnectConnection();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	printf("# Connecting to %s:%d\n", ADDRESS, PORT);

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1)
		ERREXIT("Cannot establish connection.");
}

static void showConnectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	caddr = rudpGetLocalAddress(conn);

	addressToString(caddr, strcaddr);
	
	saddr = rudpGetPeerAddress(conn);

	addressToString(saddr, strsaddr);

	printf("Connection (%ld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);
}

static void profileEcho() {
	char rcvdata[MSGSIZE];
	size_t rcvd;
	unsigned long iteration;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	printf("# Profiling echo on established connection (drop %LF%%): %lu iterations on %zu bytes...\n", DROPRATE * 100.0, ITERATIONS, MSGSIZE);

	setDropRate(DROPRATE);

	milliselaps = 0.0;

	for (iteration = 1; iteration <= ITERATIONS; iteration++) {

		printf("\nECHO %lu\n", iteration);
		
		start = getTimestamp();

		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvd = rudpReceive(conn, rcvdata, MSGSIZE);

		end = getTimestamp();

		milliselaps += getElapsed(start, end);

		assert(rcvd == MSGSIZE);

		assert(strncmp(rcvdata, MSG, MSGSIZE) == 0);

		printf("[RCVD]>%.*s\n", (int)rcvd, rcvdata);
	}

	KB = (long double)(2.0 * ITERATIONS * MSGSIZE / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Droprate: %LF%% Time: %LFs Speed: %LFKbps\n", KB, DROPRATE * 100.0, milliselaps / 1000.0, Kbps);
}
