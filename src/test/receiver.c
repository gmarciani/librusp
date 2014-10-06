#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

#define MSGSIZE 108
#define NUM_ECHOS 100

int main(int argc, char **argv) {
	ConnectionId lconn, aconn;
	struct sockaddr_in laddr, aaddr, caddr;
	char *strladdr = NULL;
	char *straaddr = NULL;
	char *strcaddr = NULL;
	char *rcvdata = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage %s [server-port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("# Opening listening connection on port: %d #\n", atoi(argv[1]));

	lconn = rudpListen(atoi(argv[1]));

	if (lconn == -1) {
		fprintf(stderr, "# Cannot open listening connection on port %d #\n", atoi(argv[1]));
		exit(EXIT_FAILURE);
	}

	printf("# Connection created in pool and listening with id: %d #\n", lconn);

	printf("# Getting local address of listening connection #\n");	

	laddr = rudpGetLocalAddress(lconn);

	strladdr = addressToString(laddr);

	printf("%s\n", strladdr);

	free(strladdr);

	printf("# Accepting incoming connection #\n");

	aconn = rudpAccept(lconn);

	printf("# Connection accepted, created in pool and established with id: %d #\n", aconn);

	printf("# Getting local address of established connection #\n");	

	aaddr = rudpGetLocalAddress(aconn);

	straaddr = addressToString(aaddr);

	printf("%s\n", straaddr);

	free(straaddr);

	printf("# Getting peer address of established connection #\n");	

	caddr = rudpGetPeerAddress(aconn);

	strcaddr = addressToString(caddr);	

	printf("%s\n", strcaddr);	

	free(strcaddr);

	printf("# Closing listening connection #\n");

	rudpClose(lconn);

	printf("# Echoing (%d times) data (%d bytes a time) on established connection #\n", NUM_ECHOS, MSGSIZE);

	int i = 0;

	for (i = 0; i < NUM_ECHOS; i++) {

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n", rcvdata);

		free(rcvdata);
	}	

	printf("# Disconnecting established connection #\n");

	rudpDisconnect(aconn);

	exit(EXIT_SUCCESS);
}
