#include <stdlib.h>
#include <stdio.h>
#include "../rudp.h"

int main(int argc, char **argv) {
	int connid;
	char *sndData, *rcvData;

	connid = rudpConnect(argv[1], atoi(argv[2]));

	sndData = getUserInput("[To Send]>");

	rudpSend(connid, sndData);

	rcvData = rudpReceive(connid, 5);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(connid);

	free(sndData);
	free(rcvData);

	exit(EXIT_SUCCESS);
}
