#include "protocol/rudp.h"

int main(int argc, char **argv) {
	rudpConnection_t conn;
	char *sndData, *rcvData;
	size_t rcvSize = 10;

	conn = rudpListen(atoi(argv[1]));	

	rudpAccept(&conn);

	rcvData = rudpReceive(&conn, rcvSize);

	printf("[Received] %s\n", rcvData);

	rudpSend(&conn, rcvData);

	rudpDisconnect(&conn);

	return(EXIT_SUCCESS);
}
