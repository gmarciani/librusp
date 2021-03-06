#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "rusp.h"

#define PORT  55000
#define LOSS  0.0
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static char address[ADDRIPV4_STR];

static int port;

static double loss;

static char filesnd[PATH_MAX];

static int debug;

static void parseArguments(int argc, char **argv);

int main(int argc, char **argv) {	
	ConnectionId conn;
	struct sockaddr_in addr, saddr;
	char straddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];
	int fd;
	char snddata[BSIZE];
	ssize_t rd;
	long long size, sent;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	parseArguments(argc, argv);

	conn = ruspConnect(address, port);

	ruspLocal(conn, &addr);

	ruspPeer(conn, &saddr);

	addressToString(addr, straddr);

	addressToString(saddr, strsaddr);

	printf("WELCOME TO ECHO CLIENT\n\n");

	printf("Running on %s\n", straddr);

	printf("Connected to %s\n", strsaddr);

	fd = openFile(filesnd, O_RDONLY);

	size = getFileSize(fd);

	printf("Sending File (size: %lld): %s\n", size, filesnd);

	sent = 0;

	milliselaps = 0.0;

	errno = 0;

	while ((rd = read(fd, snddata, BSIZE)) > 0) {
		
		start = getTimestamp();

		ruspSend(conn, snddata, rd);

		end = getTimestamp();

		milliselaps += getElapsed(start, end);

		sent += rd;

		progressBar(sent, size);

		errno = 0;
	}

	if (rd == -1)
		ERREXIT("Cannot read file: %s.", strerror(errno));

	closeFile(fd);

	printf(" OK\n");

	KB = (long double)(size / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Droprate: %F%% Time: %LFs Speed: %LFKbps\n", KB, loss * 100.0, milliselaps / 1000.0, Kbps);

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
				printf("@App:       UPLOAD client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       UPLOAD client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s [address] [file] (-p port) (-l loss) (-d)\n", argv[0]);
				printf("@Opts:      -p port: UPLOAD server port number. Default (%d) if not specified.\n", PORT);
				printf("            -l loss: Uniform probability of segments loss. Default (%F) if not specified.\n", LOSS);
				printf("            -d:      Enable debug mode. Default (%d) if not specified\n\n", DEBUG);
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

	if (optind + 2 != argc)
		ERREXIT("@Usage: %s [address] [file] (-p port) (-l loss) (-d)\n", argv[0]);

	sprintf(address,"%s", argv[optind]);

	sprintf(filesnd, "%s", argv[optind + 1]);

	ruspSetAttr(RUSP_ATTR_DROPR, &loss);

	ruspSetAttr(RUSP_ATTR_DEBUG, &debug);
}

