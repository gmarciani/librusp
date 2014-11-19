#include <stdlib.h>
#include <stdio.h>
#include "rusp.h"

#define PORT  55000
#define LOSS  0.0
#define DEBUG 0

static char address[ADDRIPV4_STR];

static int port;

static double loss;

static int debug;

static void parseArguments(int argc, char **argv);

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in addr, saddr;
	char straddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];
	char *input, rcvdata[500];
	ssize_t snd, rcv;

	parseArguments(argc, argv);

	conn = ruspConnect(address, port);

	ruspLocal(conn, &addr);

	ruspPeer(conn, &saddr);

	addressToString(addr, straddr);

	addressToString(saddr, strsaddr);

	printf("WELCOME TO ECHO CLIENT\n\n");

	printf("Running on %s\n", straddr);

	printf("Connected to %s\n", strsaddr);

	while (1) {

		input = getUserInput("[INPUT (empty to disconnect)]>");

		if ((snd = strlen(input)) == 0) {
			free(input);
			break;
		}

		ruspSend(conn, input, snd);

		if ((rcv = ruspReceive(conn, rcvdata, 500)) > 0)
			printf("[RCV] %.*s\n", (int) rcv, rcvdata);
		else
			ERREXIT("Receiving error!");

		free(input);
	}

	ruspClose(conn);

	return(EXIT_SUCCESS);
}

static void parseArguments(int argc, char **argv) {
	int opt;

	port = PORT;
	loss = LOSS;
	debug = DEBUG;

	while ((opt = getopt(argc, argv, "vhp:l:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       ECHO client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       ECHO client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s [address] (-p port) (-l loss) (-d)\n", argv[0]);
				printf("@Opts:      -p port: ECHO server port number. Default (%d) if not specified.\n", PORT);
				printf("            -l loss: Uniform probability of segments loss. Default (%F) if not specified.\n", LOSS);
				printf("            -d:      Debug mode. Default (%d) if not specified\n\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'l':
				loss = strtod(optarg, NULL);
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

	if (optind + 1 != argc)
		ERREXIT("@Usage: %s [address] (-p port) (-l loss) (-d)\n", argv[0]);

	sprintf(address, "%s", argv[optind]);

	ruspSetAttr(RUSP_ATTR_DROPR, &loss);

	ruspSetAttr(RUSP_ATTR_DEBUG, &debug);
}
