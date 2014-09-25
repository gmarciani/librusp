#include "../rudp.h"

int main(int argc, char **argv) {
	Connection conn;
	int lsock;
	char *data;

	lsock = rudpListen(atoi(argv[1]));	

	conn = rudpAccept(lsock);

	data = rudpReceive(&conn, 10);

	printf("[Received] %s\n", data);

	rudpSend(&conn, data);

	rudpDisconnect(&conn);

	free(data);

	return(EXIT_SUCCESS);
}
