#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../rudp.h"
#include "../util/sockutil.h"
#include "../util/macroutil.h"

#define ADDRESS "127.0.0.1"
#define PORT 55000

#define MSG2 "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define MSG "aaaaabbbbbcccccdddddeeeeefffff"

#define MSGSIZE strlen(MSG)

#define ITERATIONS 10
#define NDROP 0.000
#define LDROP 0.010
#define HDROP 0.050

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileEcho(const long double droprate);

int main(int argc, char **argv) {	
	
	establishConnection();

	showConnectionDetails();

	profileEcho(NDROP);

	profileEcho(LDROP);

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

	printf("Connection (%d) established on: %s with: %s.\n", conn, strcaddr, strsaddr);

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

		//printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		//printf("[RCVD]>%s\n", rcvdata);

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
