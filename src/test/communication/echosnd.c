#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define ADDRESS "127.0.0.1"
#define PORT 55000

#define MSG "aaaaabbbbbcccccdddddeeeeefffffgg"

#define MSGSIZE strlen(MSG)

#define ITERATIONS 100
#define NDROP 0.000
#define LDROP 0.050
#define HDROP 0.200

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileEcho(const long double droprate);

int main(int argc, char **argv) {	
	
	establishConnection();

	showConnectionDetails();

	//profileEcho(NDROP);

	//profileEcho(LDROP);

	profileEcho(HDROP);

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
	char *strcaddr, *strsaddr = NULL;

	caddr = rudpGetLocalAddress(conn);

	strcaddr = addressToString(caddr);
	
	saddr = rudpGetPeerAddress(conn);

	strsaddr = addressToString(saddr);		

	printf("Connection (%ld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);

	free(strcaddr);

	free(strsaddr);
}

static void profileEcho(const long double droprate) {
	struct timespec start, end;
	long double elaps, speed, sent;
	char *rcvdata = NULL;
	unsigned long iteration;

	printf("# Profiling echoing on established connection (%d iterations on %zu bytes with droprate %LF%%)\n", ITERATIONS, MSGSIZE, droprate);

	setDropRate(droprate);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 1; iteration <= ITERATIONS; iteration++) {

		printf("\nECHO %ld\n", iteration);
		
		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		free(rcvdata);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elaps = ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / 1000000000.0));

	speed = (2.0 * (long double)(ITERATIONS * MSGSIZE * 8.0 * 0.001)) / elaps;

	sent = (long double)(2.0 * ITERATIONS * MSGSIZE * 0.001);

	printf("Sent: %LFKB Droprate: %LF%% Time: %LFs Speed: %LFKbps\n", sent, droprate, elaps, speed);
}
