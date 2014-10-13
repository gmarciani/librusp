#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"
#include "../util/sockutil.h"

#define PORT 55000

#define MSG "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define MSG2 "aaaaabbbbbcccccdddddeeeee"

#define MSGSIZE strlen(MSG)

#define NUM_ECHOS 100
#define NDROP 0.000
#define LDROP 0.010
#define HDROP 0.050

#define ERREXIT(errmsg) do{fprintf(stderr, errmsg "\n");exit(EXIT_FAILURE);}while(0)

int main(void) {
	ConnectionId lconn, aconn;
	struct sockaddr_in laddr, aaddr, caddr;
	char *rcvdata, *strladdr, *straaddr, *strcaddr = NULL;
	long iteration;

	printf("# Opening listening connection on port: %d\n", PORT);

	lconn = rudpListen(PORT);

	if (lconn == -1) 
		ERREXIT("Cannot setup listening connection.");

	laddr = rudpGetLocalAddress(lconn);

	strladdr = addressToString(laddr);	

	printf("Connection (%d) listening on: %s.\n", lconn, strladdr);		

	free(strladdr);

	printf("# Accepting incoming connection\n");

	aconn = rudpAccept(lconn);

	aaddr = rudpGetLocalAddress(aconn);

	straaddr = addressToString(aaddr);

	caddr = rudpGetPeerAddress(aconn);

	strcaddr = addressToString(caddr);	

	printf("Connection (%d) established on: %s with: %s.\n", aconn, straaddr, strcaddr);		

	free(straaddr);	

	free(strcaddr);

	printf("# Closing listening connection\n");

	rudpClose(lconn);

	printf("Listening connection closed.\n");

	printf("# Echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, NDROP);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {
		
		printf("\necho %ld\n", iteration);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n", rcvdata);

		free(rcvdata);
	}	

	printf("# Echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, LDROP);

	setDropRate(LDROP);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {
		
		printf("\necho %ld\n", iteration);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n", rcvdata);

		free(rcvdata);
	}

	printf("# Echoing on established connection (%d iteration on %zu bytes with droprate %f%%)\n", NUM_ECHOS, MSGSIZE, HDROP);

	setDropRate(HDROP);

	for (iteration = 0; iteration < NUM_ECHOS; iteration++) {
		
		printf("\necho %ld\n", iteration);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n", rcvdata);

		free(rcvdata);
	}

	exit(EXIT_SUCCESS);
}
