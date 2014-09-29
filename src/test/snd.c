#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

#define RCVSIZE 5

int main(int argc, char **argv) {
	ConnectionId conn;
	char *sndData, *rcvData;

	conn = rudpConnect(argv[1], atoi(argv[2]));

	sndData = getUserInput("[To Send]>");

	rudpSend(conn, sndData);

	rcvData = rudpReceive(conn, RCVSIZE);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(conn);

	free(sndData);
	free(rcvData);

	exit(EXIT_SUCCESS);
}
