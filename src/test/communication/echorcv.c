#include <stdlib.h>
#include <stdio.h>
#include "../../rudp.h"
#include "../../util/sockutil.h"
#include "../../util/macroutil.h"

#define PORT 55000

#define MSG2 "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendus. Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat."

#define MSG "aaaaabbbbbcccccdddddeeeeefffff"

#define MSGSIZE strlen(MSG)

static ConnectionId lconn;

static ConnectionId aconn;

static void startListen(void);

static void showListeningConnectionDetails(void);

static void acceptIncomingConnection(void);

static void stopListen(void);

static void showEstablishedConnectionDetails(void);

static void echo(void);

int main(void) {	

	startListen();

	showListeningConnectionDetails();

	acceptIncomingConnection();	

	stopListen();

	showEstablishedConnectionDetails();	

	echo();	

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	printf("# Opening listening connection on port: %d\n", PORT);

	lconn = rudpListen(PORT);

	if (lconn == -1) 
		ERREXIT("Cannot setup listening connection.");
}

static void showListeningConnectionDetails(void) {
	struct sockaddr_in laddr;
	char *strladdr = NULL;

	laddr = rudpGetLocalAddress(lconn);

	strladdr = addressToString(laddr);	

	printf("Connection (%ld) listening on: %s.\n", lconn, strladdr);		

	free(strladdr);
}

static void acceptIncomingConnection(void) {
	printf("# Accepting incoming connection\n");

	aconn = rudpAccept(lconn);

	printf("SUCCESS\n");
}

static void stopListen(void) {
	printf("# Closing listening connection\n");

	rudpClose(lconn);

	printf("SUCCESS\n");
}

static void showEstablishedConnectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char *straaddr, *strcaddr = NULL;

	aaddr = rudpGetLocalAddress(aconn);

	straaddr = addressToString(aaddr);

	caddr = rudpGetPeerAddress(aconn);

	strcaddr = addressToString(caddr);	

	printf("Connection (%ld) established on: %s with: %s.\n", aconn, straaddr, strcaddr);		

	free(straaddr);	

	free(strcaddr);
}

static void echo(void) {
	char *rcvdata = NULL;
	unsigned long iteration;

	printf("# Echoing on established connection\n");

	iteration = 1;

	while (1) {
		
		printf("\nECHO %lu\n", iteration);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		if (strcmp(rcvdata, MSG) != 0)
			ERREXIT("ECHO FAILURE");

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		free(rcvdata);

		iteration++;
	}
}
