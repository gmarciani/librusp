#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

#define MSGSIZE 108
#define NUM_ECHOS 1000

#define PORT 55000

int main(void) {
	ConnectionId lconn, aconn;
	struct sockaddr_in laddr, aaddr, caddr;
	char *strladdr = NULL;
	char *straaddr = NULL;
	char *strcaddr = NULL;
	char *rcvdata = NULL;

	printf("# Opening listening connection on port: %d\n", PORT);

	lconn = rudpListen(PORT);

	if (lconn == -1) {
		fprintf(stderr, "Cannot open listening connection on port %d\n", PORT);
		exit(EXIT_FAILURE);
	}

	printf("Connection created in pool and listening with id: %d\n", lconn);

	printf("# Getting local address of listening connection\n");	

	laddr = rudpGetLocalAddress(lconn);

	strladdr = addressToString(laddr);

	printf("%s\n", strladdr);

	free(strladdr);

	printf("# Accepting incoming connection\n");

	aconn = rudpAccept(lconn);

	printf("Connection accepted, created in pool and established with id: %d\n", aconn);

	printf("# Getting local address of established connection\n");	

	aaddr = rudpGetLocalAddress(aconn);

	straaddr = addressToString(aaddr);

	printf("%s\n", straaddr);

	free(straaddr);

	printf("# Getting peer address of established connection\n");	

	caddr = rudpGetPeerAddress(aconn);

	strcaddr = addressToString(caddr);	

	printf("%s\n", strcaddr);	

	free(strcaddr);

	printf("# Closing listening connection\n");

	rudpClose(lconn);

	printf("Listening connection closed\n");

	printf("# Echoing (%d times) data (%d bytes a time) on established connection\n", NUM_ECHOS, MSGSIZE);

	long i = 0;

	for (i = 0; i < NUM_ECHOS; i++) {
		
		printf("echo %ld\n", i);

		rcvdata = rudpReceive(aconn, MSGSIZE);

		printf("[RCVD]>%s\n", rcvdata);

		rudpSend(aconn, rcvdata, strlen(rcvdata));

		printf("[SENT]>%s\n\n", rcvdata);

		free(rcvdata);
	}	

	/*printf("# Disconnecting established connection\n");

	rudpDisconnect(aconn);

	printf("Connection disconnected\n");*/

	exit(EXIT_SUCCESS);
}
