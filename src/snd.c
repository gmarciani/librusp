#include "protocol/rudp.h"

int main(int argc, char **argv) {
	rudpConn_t conn;
	char *sndData, *rcvData;

	conn = rudpConnect(argv[1], atoi(argv[2]));

	sndData = getUserInput("[To Send]>");

	rudpSend(&conn, data);

	rcvData = rudpReceive(&conn);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(&conn);

	free(sndData);
	free(rcvData);

	return(EXIT_SUCCESS);
}
