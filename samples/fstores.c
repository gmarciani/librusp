#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "rusp.h"

#define PORT  55000
#define MATCH 0
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static int port = PORT;

static int match = MATCH;

static char *filemtx;

static int debug = DEBUG;

static void parseArguments(int argc, char **argv);

static void *service(void *arg);

int main(int argc, char **argv) {
	ConnectionId lconn, conn;
	struct sockaddr_in addr;
	char straddr[ADDRIPV4_STR];

	parseArguments(argc, argv);

	lconn = ruspListen(port);

	ruspLocal(lconn, &addr);

	addressToString(addr, straddr);

	printf("WELCOME TO FILE STORE SERVER\n\n");

	printf("Running on %s\n", straddr);

	while ((conn = ruspAccept(lconn))) {

		pthread_t tid;
		ConnectionId tconn = conn;

		pthread_create(&tid, NULL, service, &tconn);
	}

	printf("Server Shutdown\n");

	ruspClose(lconn);

	return(EXIT_SUCCESS);
}

static void *service(void *arg) {
	ConnectionId conn = *((ConnectionId *) arg);
	int fdrcv, fdmtx;
	struct sockaddr_in paddr;
	char fname[1024];
	char rcvdata[BSIZE];
	ssize_t rcvd;

	ruspPeer(conn, &paddr);

	addressToString(paddr, fname);

	fdrcv = openFile(fname, O_RDWR|O_CREAT|O_TRUNC);

	while ((rcvd = ruspReceive(conn, rcvdata, BSIZE)) > 0) {

		errno = 0;
		if (write(fdrcv, rcvdata, rcvd) == -1)
			ERREXIT("Cannot write to file: %s.", strerror(errno));
	}

	ruspClose(conn);

	closeFile(fdrcv);

	if (match) {

		fdrcv = openFile(fname, O_RDONLY);

		fdmtx = openFile(filemtx, O_RDONLY);

		if (!isEqualFile(fdrcv, fdmtx))
			printf("File %s does not match File %s\n", fname, filemtx);

		closeFile(fdrcv);

		closeFile(fdmtx);
	}

	return NULL;
}

static void parseArguments(int argc, char **argv) {
	int opt;

	while ((opt = getopt(argc, argv, "vhp:m:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       FILE STORE server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       FILE STORE server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s (-p port) (-d)\n", argv[0]);
				printf("@Opts:      -p port: FILE STORE server port number. Default (%d) if not specified.\n", PORT);
				printf("            -m file: Equality match for all received files. Default (%d) if not specified.\n", MATCH);
				printf("            -d:      Debug mode. Default (%d) if not specified.\n\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'm':
				match = 1;
				memcpy(filemtx, optarg, strlen(optarg));
				break;
			case 'd':
				debug = 1;
				break;
			case '?':
				printf("Bad option %c.\n", optopt);
				printf("-h for help.\n\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}

	ruspSetAttr(RUSP_ATTR_DEBUG, &debug);
}
