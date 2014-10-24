#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/fileutil.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

static char *ADDRESS;

static int PORT;

static long double DROPRATE;

static char *FILESND;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileFileSend();

static inline void progressBar(long i, long n);

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [file] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROPRATE = strtold(argv[3], NULL);

	FILESND = argv[4];

	DEBUGMODE = atoi(argv[5]);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	profileFileSend();

	//disconnectConnection();

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
	char sndbuff[500];
	char eof = 0x01;
	int fd;
	long size, sent;
	ssize_t rd;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	fd = openFile(FILESND, O_RDONLY);

	size = getFileSize(fd);

	printf("# Profiling file send on established connection (drop: %LF%%): %s (%ld bytes)...\n", DROPRATE * 100.0, FILESND, size);

	setDropRate(DROPRATE);

	sent = 0;

	milliselaps = 0.0;

	errno = 0;

	while ((rd = read(fd, sndbuff, 500)) > 0) {
		
		start = getTimestamp();

		rudpSend(conn, sndbuff, rd);

		end = getTimestamp();

		milliselaps += getElapsed(start, end);

		memset(sndbuff, 0, sizeof(char) * 500);

		sent += rd;

		progressBar(sent, size);

		errno = 0;
	}

	if (rd == -1)
		ERREXIT("Cannot read file: %s.", strerror(errno));

	rudpSend(conn, &eof, 1);

	closeFile(fd);

	printf(" OK\n");

	KB = (long double)(size / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Droprate: %LF%% Time: %LFs Speed: %LFKbps\n", KB, DROPRATE * 100.0, milliselaps / 1000.0, Kbps);
}

static char PROG_TRUE[20] = "====================";
static char PROG_FALSE[20] = "                    ";

static inline void progressBar(long i, long n) {
	int ratio;
	int curr;

    ratio = i * 100 / n;

    if (ratio % 5 != 0)
    	return;

    printf("\r");

    fflush(stdout);

    curr = ratio / 5;

    printf("%3d%% [%.*s%.*s]", ratio, curr, PROG_TRUE, 20-curr, PROG_FALSE);
}
