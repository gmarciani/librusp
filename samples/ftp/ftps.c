#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "rusp.h"

#define PORT 55000

//int parseArguments(int argc, char **argv);

static void *service(void *arg);

int main(int argc, char **argv) {
	ConnectionId lconn, conn;
	struct sockaddr_in addr;
	char straddr[ADDRIPV4_STR];
	//message_t rqst, resp;

	//lport = parseArguments(argc, argv);

	lconn = ruspListen(PORT);

	ruspLocal(lconn, &addr);

	addressToString(addr, straddr);

	printf("WELCOME TO FTP SERVER\n\n");

	printf("Running on %s\n", straddr);

	while ((conn = ruspAccept(lconn))) {

		pthread_t tid;
		ConnectionId tconn = conn;

		pthread_create(&tid, NULL, service, &tconn);
	}

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
/*
int parseArguments(int argc, char **argv) {
	int lport = RUDI_PORT;
	int opt;

	while ((opt = getopt(argc, argv, "vhp:d:")) != -1) {
		switch (opt) {
			case 'v':
				printf("@Rudis:     RUDI FTP server based on RUDP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@Rudis:     RUDI FTP server based on RUDP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s (-p port) (-d dres)\n", argv[0]);
				printf("@Opts:      -p port: RUDI server port number. Default (%d) if not specified.\n", RUDI_PORT);
				printf("@Debug:     -d dres: Enable debug mode with the specified resolution. Valid debug resolutions are: message (m), packets (p) and datagrams (d).\n\n");
				exit(EXIT_SUCCESS);
			case 'p':
				lport = atoi(optarg);
				break;
			case 'd':
				if (strchr(optarg, 'm')) {
					rudiMessageResolution(1);
				} else if (strchr(optarg, 'p')) {
					rudpPacketResolution(1);
				} else if (strchr(optarg, 'd')) {
					_rudpDatagramResolution(1);
				} else {
					printf("Bad option -d: unrecognized debug resolution %s.\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case '?':
				printf("Bad option %c.\n", optopt);
				printf("-h for help.\n\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}	
	}

	return lport;
}*/
