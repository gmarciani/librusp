#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "rusp.h"
#include "lftpcore.h"

#define PORT 55000
#define REPO "."
#define LOSS 0.0
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static char address[ADDRIPV4_STR];

static int port;

static char repo[PATH_MAX];

static double loss;

static int debug;

static void handleMessage(Session *session, const Message response);

static void *sndFile(void *arg);

static void *rcvFile(void *arg);

static void parseArguments(int argc, char **argv);

int main(int argc, char **argv) {
	Session session;
	Message request, response;
	struct sockaddr_in addr, paddr;
	char straddr[ADDRIPV4_STR], strpaddr[ADDRIPV4_STR];
	int choice;

	parseArguments(argc, argv);

	sprintf(session.cwd, "%s", repo);

	session.ctrlconn = ruspConnect(address, port);

	session.dataconn = ruspListen(port + 1);

	ruspLocal(session.ctrlconn, &addr);

	ruspPeer(session.ctrlconn, &paddr);

	addressToString(addr, straddr);

	addressToString(paddr, strpaddr);

	printf("WELCOME TO FTP CLIENT\n\n");

	printf("Running on %s\n", straddr);

	printf("Connected to %s\n", strpaddr);

	while ((choice = runMenu(&session, &request)) != LFTP_MENU_EXIT) {

		if (choice == LFTP_MENU_ERROR)
			continue;

		sendMessage(session.ctrlconn, request);

		receiveMessage(session.ctrlconn, &response);

		handleMessage(&session, response);
	}

	ruspClose(session.dataconn);

	ruspClose(session.ctrlconn);

	printf("Disconnected\n");

	return(EXIT_SUCCESS);
}

static void handleMessage(Session *session, const Message response) {
	char paths[2][PATH_MAX];
	char **params;
	int nparams, i;
	DataTransfer *transfer;
	pthread_t tid;

	switch (response.header.type) {
		case LFTP_SUCCESS:
			switch (response.header.action) {
				case LFTP_GTCWD:
					printf("[ANSWER]>CWD: %s\n", response.body);
					break;
				case LFTP_CHDIR:
					printf("[SUCCESS]>CWD: %s\n", response.body);
					break;
				case LFTP_LSDIR:
					params = arrayDeserialization(response.body, LFTP_PDELIM, &nparams);
					printf("[SUCCESS]>Listing %s\n", params[0]);
					for (i = 1; i < nparams; i++)
						printf("%s\n", params[i]);
					for (i = 0; i < nparams; i++)
						free(params[i]);
					free(params);
					break;
				case LFTP_MKDIR:
					printf("[SUCCESS]>Directory created %s\n", response.body);
					break;
				case LFTP_RMDIR:
					printf("[SUCCESS]>Directory removed %s\n", response.body);
					break;
				case LFTP_CPDIR:
					params = arrayDeserialization(response.body, LFTP_PDELIM, &nparams);
					printf("[SUCCESS]>Directory %s copied to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case LFTP_MVDIR:
					params = arrayDeserialization(response.body, LFTP_PDELIM, &nparams);
					printf("[SUCCESS]>Directory %s moved to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case LFTP_RETRF:
					transfer = malloc(sizeof(DataTransfer));
					transfer->conn = ruspAccept(session->dataconn);
					getFilename(response.body, paths[0]);
					sprintf(paths[1], "%s/%s", session->cwd, paths[0]);
					transfer->fd = open(paths[1], O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
					pthread_create(&tid, NULL, rcvFile, transfer);
					printf("[SUCCESS]>Downloading %s\n", response.body);
					break;
				case LFTP_STORF:
					transfer = malloc(sizeof(DataTransfer));
					transfer->conn = ruspAccept(session->dataconn);
					transfer->fd = open(response.body, O_RDONLY);
					pthread_create(&tid, NULL, sndFile, transfer);
					printf("[SUCCESS]>Uploading %s\n", response.body);
					break;
				case LFTP_RMFIL:
					printf("[SUCCESS]>File removed %s\n", response.body);
					break;
				case LFTP_CPFIL:
					params = arrayDeserialization(response.body, LFTP_PDELIM, &nparams);
					printf("[SUCCESS]>File %s copied to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case LFTP_MVFIL:
					params = arrayDeserialization(response.body, LFTP_PDELIM, &nparams);
					printf("[SUCCESS]>File %s moved to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				default:
					break;
			}
			break;
		case LFTP_BADRQST:
			printf("[BADRQST]>%s\n", response.body);
			break;
		default:
			break;
	}
}

static void *sndFile(void *arg) {
	DataTransfer *upload = (DataTransfer *) arg;
	char data[BSIZE];
	ssize_t rd;

	while ((rd = read(upload->fd, data, BSIZE)) > 0)
		ruspSend(upload->conn, data, rd);

	ruspClose(upload->conn);

	close(upload->fd);

	free(upload);

	return NULL;
}

static void *rcvFile(void *arg) {
	DataTransfer *download = (DataTransfer *) arg;
	char data[BSIZE];
	ssize_t rcvd, wr;

	while ((rcvd = ruspReceive(download->conn, data, BSIZE)) > 0) {
		errno = 0;
		if ((wr = write(download->fd, data, rcvd)) != rcvd)
			ERREXIT("Cannot write to file: %s", strerror(errno));
	}

	ruspClose(download->conn);

	close(download->fd);

	free(download);

	return NULL;
}

static void parseArguments(int argc, char **argv) {
	int opt;

	port = PORT;
	sprintf(repo, "%s", REPO);
	loss = LOSS;
	debug = DEBUG;

	while ((opt = getopt(argc, argv, "vhp:r:l:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       LFTP client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       LFTP client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s [address] (-p port) (-r repo) (-l loss) (-d)\n", argv[0]);
				printf("@Opts:      -p port: LFTP server port number. Default (%d) if not specified.\n", PORT);
				printf("            -r repo: LFTP client repository. Default (%s) if not specified.\n", REPO);
				printf("            -l loss: Uniform probability of segments loss. Default (%F) if not specified.\n", LOSS);
				printf("            -d     : Debug mode. Default (%d) if not specified.\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				memcpy(repo, optarg, strlen(optarg));
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
		ERREXIT("@Usage: %s [address] (-p port) (-r repo) (-l loss) (-d)\n", argv[0]);

	sprintf(address, "%s", argv[optind]);

	ruspSetAttr(RUSP_ATTR_DROPR, &loss);

	LFTP_DEBUG = debug;
}
