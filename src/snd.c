#include "protocol/rudp.h"

int main(int argc, char **argv) {
	struct sockaddr_in saddr;
	rudpConnection_t conn;
	char *sndData, *rcvData;
	size_t rcvSize = 10;

	saddr = rudpAddress(argv[1], atoi(argv[2]));

	conn = rudpConnect(saddr);

	sndData = getUserInput("[To Send]>");

	rudpSend(&conn, data);

	rcvData = rudpReceive(&conn, rcvSize);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(&conn);

	free(sndData);
	free(rcvData);

	return(EXIT_SUCCESS);
}
