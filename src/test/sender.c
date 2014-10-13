#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../rudp.h"
#include "../util/sockutil.h"

#define ADDRESS "127.0.0.1"
#define PORT 55000

#define MSG "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define MSG2 "aaaaabbbbbcccccdddddeeeee"

#define MSGSIZE strlen(MSG)

#define NUM_ECHOS 100
#define NDROP 0.000
#define LDROP 0.010
#define HDROP 0.050

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in caddr, saddr;
	struct timespec start, end;
	double elapsN, elapsL, elapsH;
	double speedN, speedL, speedH;
	char *rcvdata, *strcaddr, *strsaddr = NULL;
	long iteration;

	printf("# Connecting to %s:%d\n", ADDRESS, PORT);

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1)
		ERREXIT("Cannot establish connection.");

	caddr = rudpGetLocalAddress(conn);

	strcaddr = addressToString(caddr);
	
	saddr = rudpGetPeerAddress(conn);

	strsaddr = addressToString(saddr);		

	printf("Connection (%d) established on: %s with: %s.\n", conn, strcaddr, strsaddr);

	free(strcaddr);

	free(strsaddr);

	printf("# Profiling echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, NDROP);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {

		printf("\necho %ld\n", iteration);
		
		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		free(rcvdata);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsN = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	speedN = (2.0 * (double) (NUM_ECHOS * MSGSIZE * 8.0 * 0.001)) / elapsN;

	printf("# Profiling echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, LDROP);

	setDropRate(LDROP);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {

		printf("\necho %ld\n", iteration);
		
		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		free(rcvdata);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsL = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	speedL = (2.0 * (double) (NUM_ECHOS * MSGSIZE * 8.0 * 0.001)) / elapsL;

	printf("# Profiling echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, HDROP);

	setDropRate(HDROP);

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {

		printf("\necho %ld\n", iteration);
		
		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		free(rcvdata);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsH = (end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

	speedH = (2.0 * (double) (NUM_ECHOS * MSGSIZE * 8.0 * 0.001)) / elapsH;

	printf("\nSummary:\n");
	printf("Sent: %f KB\n", (double) 2.0 * NUM_ECHOS * MSGSIZE * 0.001);
	printf("Droprate: %f%% Time: %fs Speed: %fKbps\n", NDROP, elapsN, speedN);
	printf("Droprate: %f%% Time: %fs Speed: %fKbps\n", LDROP, elapsL, speedL);
	printf("Droprate: %f%% Time: %fs Speed: %fKbps\n", HDROP, elapsH, speedH);

	exit(EXIT_SUCCESS);
}
