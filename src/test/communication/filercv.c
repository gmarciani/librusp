#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "../../rusp.h"
#include "../../util/fileutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static int PORT;

static char *FILERCV;

static char *FILEMTX;

static int DBG;

static ConnectionId LCONN;

static ConnectionId CONN;

static void startListen(void);

static void listenDetails(void);

static void acceptConnection(void);

static void stopListen(void);

static void connectionDetails(void);

static void receiveFile(void);

static void closeConnection(void);

static void matchReceivedFile(void);

int main(int argc, char **argv) {
	int DBGON = 1;
	int DBGOFF = 0;

	if (argc < 5)
		ERREXIT("usage: %s [port] [filercv] [filemtx] [debug]", argv[0]);

	PORT = atoi(argv[1]);

	FILERCV = argv[2];

	FILEMTX = argv[3];

	DBG = atoi(argv[4]);

	if (DBG & DBG_OPEN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	startListen();

	listenDetails();

	acceptConnection();	

	stopListen();

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	connectionDetails();

	if (DBG & DBG_TRAN)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	receiveFile();

	ruspSetAttr(RUSP_ATTR_DROPR, &DBGOFF);

	if (DBG & DBG_CLOS)
		ruspSetAttr(RUSP_ATTR_DROPR, &DBGON);

	closeConnection();

	matchReceivedFile();

	exit(EXIT_SUCCESS);
}

static void startListen(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Opening listening connection on port: %d...%s", PORT, dbg?"\n":"");

	LCONN = ruspListen(PORT);

	if (LCONN == -1) 
		ERREXIT("Cannot setup listening connection.");

	printf("OK\n");
}

static void listenDetails(void) {
	struct sockaddr_in laddr;
	char strladdr[ADDRIPV4_STR];

	ruspLocal(LCONN, &laddr);

	addressToString(laddr, strladdr);

	printf("Connection (%lld) listening on: %s.\n", LCONN, strladdr);
}

static void acceptConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Accepting incoming connection...%s", dbg?"\n":"");

	CONN = ruspAccept(LCONN);

	printf("OK\n");
}

static void stopListen(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing listening connection...%s", dbg?"\n":"");

	ruspClose(LCONN);

	printf("OK\n");
}

static void connectionDetails(void) {
	struct sockaddr_in aaddr, caddr;
	char straaddr[ADDRIPV4_STR], strcaddr[ADDRIPV4_STR];

	ruspLocal(CONN, &aaddr);

	addressToString(aaddr, straaddr);

	ruspPeer(CONN, &caddr);

	addressToString(caddr, strcaddr);

	printf("Connection (%lld) established on: %s with: %s.\n", CONN, straaddr, strcaddr);
}

static void receiveFile(void) {
	char rcvdata[500];
	ssize_t rcvd;
	int fdrcv;

	fdrcv = openFile(FILERCV, O_RDWR|O_CREAT|O_TRUNC);

	printf("# Receiving file on established connection...");

	while ((rcvd = ruspReceive(CONN, rcvdata, 500)) > 0) {

		errno = 0;
		if (write(fdrcv, rcvdata, rcvd) == -1)
			ERREXIT("Cannot write to file: %s.", strerror(errno));

		memset(rcvdata, 0, sizeof(char) * 500);
	}

	printf("OK\n");

	printf("Stop receiving on established connection...OK\n");

	closeFile(fdrcv);
}

static void closeConnection(void) {
	int dbg;

	ruspGetAttr(RUSP_ATTR_DEBUG, &dbg);

	printf("# Closing established connection...%s", dbg?"\n":"");

	ruspClose(CONN);

	printf("OK\n");
}

static void matchReceivedFile(void) {
	int fdrcv, fdmtx;

	printf("# Matching received file (%s) with matrix file (%s)...", FILERCV, FILEMTX);

	fdrcv = openFile(FILERCV, O_RDONLY);

	fdmtx = openFile(FILEMTX, O_RDONLY);

	assert(isEqualFile(fdrcv, fdmtx));

	closeFile(fdrcv);

	closeFile(fdmtx);

	printf("OK\n");
}
