#include "protocol/rudp.h"

#define RCV_SIZE 10

int main(int argc, char **argv) {
	rudpConn_t conn;
	int lsock;
	char *sndData, *rcvData;

	lsock = rudpListen(atoi(argv[1]));	

	conn = rudpAccept(lsock);

	rcvData = rudpReceive(&conn, RCV_SIZE);

	printf("[Received] %s\n", rcvData);

	rudpSend(&conn, rcvData);

	rudpDisconnect(&conn);

	return(EXIT_SUCCESS);
}
