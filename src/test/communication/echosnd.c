#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../rusp.h"
#include "../../util/timeutil.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

#define MSG "aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkklllllmmmmmnnnnnooooopppppqqqqqrrrrrssssstttttuuuuuvvvvvwwwwwxxxxxyyyyyzz"

#define MSGS strlen(MSG)

static char *ADDRESS;

static int PORT;

static double DROP;

static long long ITER;

static int DBG;

static ConnectionId CONN;

static void establishConnection(void);

static void connectionDetails(void);

static void echo();

static void closeConnection(void);

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [iterations] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROP = strtod(argv[3], NULL);

	ITER = strtoll(argv[4], NULL, 10);

	DBG = atoi(argv[5]);

	rudpSetDrop(DROP);

	if (DBG & DBG_OPEN)
		rudpSetDebug(1);

	establishConnection();

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

static void establishConnection(void) {
	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, (rudpGetDebug())?"\n":"");

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

	printf("Connection (%lld) established on: %s with: %s.\n", CONN, strcaddr, strsaddr);
}

static void echo() {
	char rcvdata[MSGS];
	ssize_t rcvd;
	long long iteration;

	printf("# Sending for echo on established connection (drop %F%%): %lld iterations on %zu bytes...\n", rudpGetDrop() * 100.0, ITER, MSGS);

	for (iteration = 1; iteration <= ITER; iteration++) {

		printf("\n%lld\n", iteration);

		ruspSend(CONN, MSG, MSGS);

		printf("[SND]>%.*s\n", (int) MSGS, MSG);

		rcvd = ruspReceive(CONN, rcvdata, MSGS);

		printf("[RCV]>%.*s\n", (int) MSGS, rcvdata);

		assert(rcvd == MSGS);

		assert(strncmp(rcvdata, MSG, MSGS) == 0);

		memset(rcvdata, 0, sizeof(char) * MSGS);
	}

	printf("# Stop sending on established connection...OK\n");
}

static void closeConnection(void) {
	printf("# Closing established connection...%s", (rudpGetDebug())?"\n":"");

	ruspClose(CONN);

	printf("OK\n");
}
