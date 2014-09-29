#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

#define RCVSIZE 10

int main(int argc, char **argv) {
	ConnectionId lconn, aconn;
	char *data;

	lconn = rudpListen(atoi(argv[1]));	

	aconn = rudpAccept(lconn);

	rudpClose(lconn);

	data = rudpReceive(aconn, RCVSIZE);

	printf("[Received] %s\n", data);

	rudpSend(aconn, data);

	rudpDisconnect(aconn);

	free(data);

	exit(EXIT_SUCCESS);
}
