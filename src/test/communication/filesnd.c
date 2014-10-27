#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "../../rudp.h"
#include "../../util/cliutil.h"
#include "../../util/fileutil.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

static char *ADDRESS;

static int PORT;

static double DROPRATE;

static char *FILESND;

static int DEBUGMODE;

static ConnectionId conn;

static void establishConnection(void);

static void showConnectionDetails(void);

static void profileFileSend();

int main(int argc, char **argv) {	
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [file] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROPRATE = strtod(argv[3], NULL);

	FILESND = argv[4];

	DEBUGMODE = atoi(argv[5]);

	setDropRate(DROPRATE);

	setConnectionDebugMode(DEBUGMODE);

	establishConnection();

	showConnectionDetails();

	profileFileSend();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, (DEBUGMODE)?"\n":"");

	conn = rudpConnect(ADDRESS, PORT);

	if (conn == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void showConnectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	caddr = rudpGetLocalAddress(conn);

	addressToString(caddr, strcaddr);
	
	saddr = rudpGetPeerAddress(conn);

	addressToString(saddr, strsaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", conn, strcaddr, strsaddr);
}

static void profileFileSend() {
	char snddata[500];
	int fd;
	long long size, sent;
	ssize_t rd;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	fd = openFile(FILESND, O_RDONLY);

	size = getFileSize(fd);

	printf("# Profiling file send on established connection (drop: %F%%): %s (%lld bytes)...\n", DROPRATE * 100.0, FILESND, size);

	sent = 0;

	milliselaps = 0.0;

	errno = 0;

	while ((rd = read(fd, snddata, 500)) > 0) {
		
		start = getTimestamp();

		rudpSend(conn, snddata, rd);

		end = getTimestamp();

		milliselaps += getElapsed(start, end);

		memset(snddata, 0, sizeof(char) * 500);

		sent += rd;

		progressBar(sent, size);

		errno = 0;
	}

	if (rd == -1)
		ERREXIT("Cannot read file: %s.", strerror(errno));

	closeFile(fd);

	printf(" OK\n");

	printf("# Stop sending on established connection...OK\n");

	KB = (long double)(size / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Droprate: %F%% Time: %LFs Speed: %LFKbps\n", KB, DROPRATE * 100.0, milliselaps / 1000.0, Kbps);

	printf("# Closing established connection...%s", (DEBUGMODE)?"\n":"");

	rudpClose(conn);

	printf("OK\n");
}
