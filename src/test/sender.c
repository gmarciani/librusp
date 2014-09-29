#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in caddr, saddr;
	char *strcaddr = NULL;
	char *strsaddr = NULL;

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

	strcaddr = rudpAddressToString(caddr);

	printf("%s\n", strcaddr);

	free(strcaddr);

	printf("# Getting peer address of established connection #\n");

	saddr = rudpGetPeerAddress(conn);

	printf("# Getting string representation of peer address of established connection #\n");

	strsaddr = rudpAddressToString(saddr);	
	
	printf("%s\n", strsaddr);

	free(strsaddr);

	exit(EXIT_SUCCESS);
}
