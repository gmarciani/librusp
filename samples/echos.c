#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "rusp.h"

#define PORT 55000
#define DBG 0

static int port = PORT;

static int dbg = DBG;

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

	printf("WELCOME TO ECHO SERVER\n\n");

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
	char rcvdata[500];
	ssize_t rcvd;

	while ((rcvd = ruspReceive(conn, rcvdata, 500)) > 0) {

		ruspSend(conn, rcvdata, rcvd);

		memset(rcvdata, 0, sizeof(char) * 500);
	}

	ruspClose(conn);

	return NULL;

}

static void parseArguments(int argc, char **argv) {
	int opt;

	while ((opt = getopt(argc, argv, "vhp:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       ECHO server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       ECHO server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s (-p port) (-d)\n", argv[0]);
				printf("@Opts:      -p port: ECHO server port number. Default (%d) if not specified.\n", PORT);
				printf("            -d:      Enable debug mode. Default (%d) if not specified.\n\n", DBG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'd':
				dbg = 1;
				break;
			case '?':
				printf("Bad option %c.\n", optopt);
				printf("-h for help.\n\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}

	ruspSetAttr(RUSP_ATTR_DEBUG, &dbg);
}
