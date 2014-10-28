#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/cliutil.h"
#include "../../util/timeutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define MSG "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnooooopppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzz"

#define MSGSIZE strlen(MSG)

static char *ADDRESS;

static int PORT;

static double DROPRATE;

static long long ITERATIONS;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void echo();

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [iterations] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROPRATE = strtod(argv[3], NULL);

	ITERATIONS = strtoll(argv[4], NULL, 10);

	DEBUGMODE = atoi(argv[5]);

	setDropRate(DROPRATE);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	echo();

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

	rudpGetLocalAddress(conn, &caddr);

	addressToString(caddr, strcaddr);
	
	rudpGetPeerAddress(conn, &saddr);

	addressToString(saddr, strsaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);
}

static void echo() {
	char rcvdata[MSGSIZE];
	ssize_t rcvd;
	long long iteration;

	printf("# Sending for echo on established connection (drop %F%%): %lld iterations on %zu bytes...\n", DROPRATE * 100.0, ITERATIONS, MSGSIZE);

	for (iteration = 1; iteration <= ITERATIONS; iteration++) {

		printf("\n%lld\n", iteration);

		rudpSend(conn, MSG, MSGSIZE);

		printf("[SND]>%.*s\n", (int) MSGSIZE, MSG);

		rcvd = rudpReceive(conn, rcvdata, MSGSIZE);

		printf("[RCV]>%.*s\n", (int) MSGSIZE, rcvdata);

		assert(rcvd == MSGSIZE);

		assert(strncmp(rcvdata, MSG, MSGSIZE) == 0);

		memset(rcvdata, 0, sizeof(char) * MSGSIZE);

		//progressBar(iteration, ITERATIONS);
	}

	printf("# Stop sending on established connection...OK\n");

	printf("# Closing established connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(conn);

	printf("OK\n");
}
