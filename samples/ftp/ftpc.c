#include <stdlib.h>
#include <stdio.h>
#include "rusp.h"

#define PORT 55000
#define ADDR "192.168.1.121"

//struct sockaddr_in parseArguments(int argc, char **argv);

int main(int argc, char **argv) {
	ConnectionId conn;
	struct sockaddr_in addr, saddr;
	char straddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];
	char *input, rcvdata[500];
	ssize_t snd, rcv;
	//message_t rqst, resp;

	//saddr = parseArguments(argc, argv);

	conn = ruspConnect(ADDR, PORT);

	ruspLocal(conn, &addr);

	ruspPeer(conn, &saddr);

	addressToString(addr, straddr);

	addressToString(saddr, strsaddr);

	printf("WELCOME TO FTP CLIENT\n\n");

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
/*
struct sockaddr_in parseArguments(int argc, char **argv) {
	struct sockaddr_in saddr;
	int sport = RUDI_PORT;
	int opt;	

	while ((opt = getopt(argc, argv, "vhp:d:")) != -1) {
		switch (opt) {
			case 'v':
				printf("@Rudic:     RUDI FTP client based on RUDP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@Rudic:     RUDI FTP client based on RUDP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s [address] (-p port) (-d dres)\n", argv[0]);
				printf("@Args:      address: RUDI server ip address.\n");
				printf("@Opts:      -p port: RUDI server port number. Default (%d) if not specified.\n", RUDI_PORT);
				printf("@Debug:     -d dres: Enable debug mode with the specified resolution. Valid debug resolutions are: message (m), packets (p) and datagrams (d).\n\n");
				exit(EXIT_SUCCESS);
			case 'p':
				sport = atoi(optarg);
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
				break;
			default:
				break;
		}	
	}

	if (optind >= argc) {
		printf("Missing RUDI server address.\n");
		printf("-h for help.\n\n");
		exit(EXIT_FAILURE);
	}

	saddr = rudpAddress(argv[optind], sport);

	return saddr;
}*/
