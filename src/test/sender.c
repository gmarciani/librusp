#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in caddr, saddr;
	char *strcaddr = NULL;
	char *strsaddr = NULL;

	if (argc != 3) {
		fprintf(stderr, "Usage %s [server-ip] [server-port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("# Connecting to %s:%d #\n", argv[1], atoi(argv[2]));

	conn = rudpConnect(argv[1], atoi(argv[2]));

	if (conn == -1) {
		fprintf(stderr, "# Cannot connect to %s:%d #\n", argv[1], atoi(argv[2]));
		exit(EXIT_FAILURE);
	}

	printf("# Connection established with id: %d #\n", conn);

	printf("# Getting local address of established connection #\n");

	caddr = rudpGetLocalAddress(conn);

	printf("# Getting string representation of local address of established connection #\n");

	strcaddr = addressToString(caddr);

	printf("%s\n", strcaddr);

	free(strcaddr);

	printf("# Getting peer address of established connection #\n");

	saddr = rudpGetPeerAddress(conn);

	printf("# Getting string representation of peer address of established connection #\n");

	strsaddr = addressToString(saddr);	
	
	printf("%s\n", strsaddr);

	free(strsaddr);

	printf("# Disconnecting established connection #\n");

	rudpDisconnect(conn);

	exit(EXIT_SUCCESS);
}
