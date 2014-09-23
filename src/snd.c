#include "protocol/rudp.h"

#define RCV_SIZE 20

int main(int argc, char **argv) {
	rudpConn_t conn;
	char *sndData, *rcvData;

	conn = rudpConnect(argv[1], atoi(argv[2]));

	sndData = getUserInput("[To Send]>");

	rudpSend(&conn, data);

	rcvData = rudpReceive(&conn, RCV_SIZE);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(&conn);

	free(sndData);
	free(rcvData);

	return(EXIT_SUCCESS);
}
