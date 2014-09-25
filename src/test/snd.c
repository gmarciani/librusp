#include "../rudp.h"

int main(int argc, char **argv) {
	Connection conn;
	char *sndData, *rcvData;

	conn = rudpConnect(argv[1], atoi(argv[2]));

	sndData = getUserInput("[To Send]>");

	rudpSend(&conn, sndData);

	rcvData = rudpReceive(&conn, 5);

	printf("[Received]> %s\n", rcvData);

	rudpDisconnect(&conn);

	free(sndData);
	free(rcvData);

	return(EXIT_SUCCESS);
}
