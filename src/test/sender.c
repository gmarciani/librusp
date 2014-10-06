#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

#define ADDRESS "127.0.0.1"
#define PORT 55000

#define MSG "Hy folks! I'm Jack, and I hope you will enjoy RUDP: the new reliable transport protocol built on top of UDP!"
#define MSGSIZE 108
#define NUM_ECHOS 1000

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in caddr, saddr;
	char *rcvdata = NULL;
	char *strcaddr = NULL;
	char *strsaddr = NULL;

	printf("# Connecting to %s:%d\n", ADDRESS, PORT);

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1) {
		fprintf(stderr, "Cannot connect to %s:%d\n", ADDRESS, PORT);
		exit(EXIT_FAILURE);
	}

	printf("Connection created in pool and established with id: %d\n", conn);

	printf("# Getting local address of established connection\n");

	caddr = rudpGetLocalAddress(conn);

	strcaddr = addressToString(caddr);

	printf("%s\n", strcaddr);

	free(strcaddr);

	printf("# Getting peer address of established connection\n");

	saddr = rudpGetPeerAddress(conn);

	strsaddr = addressToString(saddr);	
	
	printf("%s\n", strsaddr);

	free(strsaddr);

	printf("# Sending (%d times) data (%d bytes a time) on established connection: %s\n", NUM_ECHOS, MSGSIZE, MSG);

	long i;

	for (i = 0; i < NUM_ECHOS; i++) {

		printf("echo %ld\n", i);
		
		rudpSend(conn, MSG, MSGSIZE);

		printf("[SENT]>%s\n", MSG);

		rcvdata = rudpReceive(conn, MSGSIZE);

		printf("[RCVD]>%s\n\n", rcvdata);

		if (strcmp(rcvdata, MSG) != 0) {

			free(rcvdata);
			
			fprintf(stderr, "ECHO FAILED!\n");

			exit(EXIT_FAILURE);
		}

		free(rcvdata);
	}

	fprintf(stderr, "\a");

	printf("ECHO SUCCEED!\n");

	/*printf("# Disconnecting established connection\n");

	rudpDisconnect(conn);

	printf("Connection disconnected\n");*/

	exit(EXIT_SUCCESS);
}
