#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "../../rusp.h"
#include "../../util/cliutil.h"
#include "../../util/fileutil.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static char *ADDRESS;

static int PORT;

static double DROP;

static char *FILESND;

static int DBG;

static ConnectionId CONN;

static void establishConnection(void);

static void connectionDetails(void);

static void sendFile();

static void closeConnection(void);

int main(int argc, char **argv) {	
	int DBGON = 1;
	int DBGOFF = 0;
	
	if (argc < 6)
		ERREXIT("usage: %s [address] [port] [drop] [file] [debug]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	DROP = strtod(argv[3], NULL);

	FILESND = argv[4];

	DBG = atoi(argv[5]);

	ruspSetAttr(RUSP_ATTR_DROPR, &DROP);

	if (DBG & DBG_OPEN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	establishConnection();

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	connectionDetails();

	if (DBG & DBG_TRAN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	sendFile();

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	if (DBG & DBG_CLOS)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	closeConnection();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Connecting to %s:%d...%s", ADDRESS, PORT, dbg?"\n":"");

	CONN = ruspConnect(ADDRESS, PORT);

	if (CONN == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in caddr, saddr;
	char strcaddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];

	ruspLocal(CONN, &caddr);

	addressToString(caddr, strcaddr);
	
	ruspPeer(CONN, &saddr);

	addressToString(saddr, strsaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", CONN, strcaddr, strsaddr);
}

static void sendFile() {
	char snddata[500];
	int fd;
	long long size, sent;
	ssize_t rd;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;
	double drop;

	ruspGetAttr(RUSP_ATTR_DROPR, &drop);

	fd = openFile(FILESND, O_RDONLY);

	size = getFileSize(fd);

	printf("# Profiling file send on established connection (drop: %F%%): %s (%lld bytes)...\n", drop * 100.0, FILESND, size);

	sent = 0;

	milliselaps = 0.0;

	errno = 0;

	while ((rd = read(fd, snddata, 500)) > 0) {
		
		start = getTimestamp();

		ruspSend(CONN, snddata, rd);

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

	printf("Sent: %LFKB Droprate: %F%% Time: %LFs Speed: %LFKbps\n", KB, drop * 100.0, milliselaps / 1000.0, Kbps);
}

static void closeConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing established connection...%s", dbg?"\n":"");

	ruspClose(CONN);

	printf("OK\n");
}
