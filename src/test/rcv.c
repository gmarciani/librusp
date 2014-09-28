#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

int main(int argc, char **argv) {
	int connid;
	int lsock;
	char *data;

	lsock = rudpListen(atoi(argv[1]));	

	connid = rudpAccept(lsock);

	data = rudpReceive(connid, 10);

	printf("[Received] %s\n", data);

	rudpSend(connid, data);

	rudpDisconnect(connid);

	free(data);

	exit(EXIT_SUCCESS);
}
