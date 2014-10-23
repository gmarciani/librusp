#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/fileutil.h"
#include "../../util/timeutil.h"

static char *ADDRESS;

static int PORT;

static long double DROPRATE;

static char *FILESND;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileFileSend();

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [file] [debug]", argv[0]);

	ADDRESS = stringDuplication(argv[1]);

	PORT = atoi(argv[2]);

	DROPRATE = strtold(argv[3], NULL);

	FILESND = stringDuplication(argv[4]);

	DEBUGMODE = atoi(argv[5]);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	profileFileSend();

	//disconnectConnection();

	free(ADDRESS);

	free(FILESND);

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	printf("# Connecting to %s:%d\n", ADDRESS, PORT);

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1)
		ERREXIT("Cannot establish connection.");
}

static void showConnectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	caddr = rudpGetLocalAddress(conn);

	addressToString(caddr, strcaddr);
	
	saddr = rudpGetPeerAddress(conn);

	addressToString(saddr, strsaddr);

	printf("Connection (%ld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);
}

static void profileFileSend() {
	char sndbuff[1024];
	int fd;
	long size;
	ssize_t rd;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	fd = openFile(FILESND, O_RDONLY);

	size = getFileSize(fd);

	printf("# Profiling file send on established connection (drop: %LF%%): %s (%ld bytes)...", DROPRATE * 100.0, FILESND, size);

	setDropRate(DROPRATE);

	milliselaps = 0.0;

	start = getTimestamp();

	while ((rd = read(fd, sndbuff, 1024)) > 0) {
		
		rudpSend(conn, sndbuff, rd);

		memset(sndbuff, 0, sizeof(char) * 1024);
	}

	end = getTimestamp();

	closeFile(fd);

	printf("OK\n");

	milliselaps += getElapsed(start, end);

	KB = (long double)(size / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Droprate: %LF%% Time: %LFs Speed: %LFKbps\n", KB, DROPRATE * 100.0, milliselaps / 1000.0, Kbps);
}
