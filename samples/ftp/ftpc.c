#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "rusp.h"
#include "ftpcore.h"

#define PORT 55000
#define REPO "."
#define LOSS 0.0
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static char *address;

static int port = PORT;

static char repo[PATH_MAX] = REPO;

static double drop = LOSS;

static int dbg = DEBUG;

static void handleMessage(Session *session, const Message response);

static void *sndFile(void *arg);

static void *rcvFile(void *arg);

static void parseArguments(int argc, char **argv);

int main(int argc, char **argv) {
	Session session;
	Message outmsg, inmsg;
	struct sockaddr_in addr, saddr;
	char straddr[ADDRIPV4_STR], strsaddr[ADDRIPV4_STR];
	int choice;

	parseArguments(argc, argv);

	session.ctrlconn = ruspConnect(address, port);

	session.dataconn = ruspListen(port + 1);

	memcpy(session.cwd, repo, strlen(repo));

	ruspLocal(session.ctrlconn, &addr);

	ruspPeer(session.ctrlconn, &saddr);

	addressToString(addr, straddr);

	addressToString(saddr, strsaddr);

	printf("WELCOME TO FTP CLIENT\n\n");

	printf("Running on %s\n", straddr);

	printf("Connected to %s\n", strsaddr);

	while ((choice = runMenu(&session, &outmsg)) != MENU_EXIT) {

		if (choice == MENU_ERROR)
			continue;

		sendMessage(session.ctrlconn, outmsg);

		receiveMessage(session.ctrlconn, &inmsg);

		handleMessage(&session, inmsg);
	}

	ruspClose(session.dataconn);

	ruspClose(session.ctrlconn);

	printf("Disconnected");

	return(EXIT_SUCCESS);
}

static void handleMessage(Session *session, const Message response) {
	char paths[2][PATH_MAX];
	char **params;
	int nparams, i;
	DataTransfer download, upload;
	pthread_t upltid, dwltid;

	switch (response.header.type) {
		case MSG_SUCCESS:
			switch (response.header.action) {
				case MSG_GTCWD:
					printf("[ANSWER]>CWD: %s\n", response.body);
					break;
				case MSG_CHDIR:
					printf("[SUCCESS]>CWD: %s\n", response.body);
					break;
				case MSG_LSDIR:
					params = arrayDeserialization(response.body, OBJECT_FIELDS_DELIMITER, &nparams);
					printf("[SUCCESS]>Listing %s\n", response.body);
					for (i = 0; i < nparams; i++)
						printf("%s\n", params[i]);
					for (i = 0; i < nparams; i++)
						free(params[i]);
					free(params);
					break;
				case MSG_MKDIR:
					printf("[SUCCESS]>Directory created %s\n", response.body);
					break;
				case MSG_RMDIR:
					printf("[SUCCESS]>Directory removed %s\n", response.body);
					break;
				case MSG_CPDIR:
					params = arrayDeserialization(response.body, OBJECT_FIELDS_DELIMITER, &nparams);
					printf("[SUCCESS]>Directory %s copied to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case MSG_MVDIR:
					params = arrayDeserialization(response.body, OBJECT_FIELDS_DELIMITER, &nparams);
					printf("[SUCCESS]>Directory %s moved to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case MSG_RETRF:
					printf("[SUCCESS]>Downloading %s\n", response.body);
					download.conn = ruspAccept(session->dataconn);;
					sprintf(download.path, "%s/%s", session->cwd, response.body);
					pthread_create(&dwltid, NULL, rcvFile, &download);
					break;
				case MSG_STORF:
					printf("[SUCCESS]>Uploading %s\n", response.body);
					upload.conn = ruspAccept(session->dataconn);
					sprintf(upload.path, "%s", response.body);
					pthread_create(&upltid, NULL, sndFile, &upload);
					break;
				case MSG_RMFIL:
					printf("[SUCCESS]>File removed %s\n", response.body);
					break;
				case MSG_CPFIL:
					params = arrayDeserialization(response.body, OBJECT_FIELDS_DELIMITER, &nparams);
					printf("[SUCCESS]>File %s copied to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				case MSG_MVFIL:
					params = arrayDeserialization(response.body, OBJECT_FIELDS_DELIMITER, &nparams);
					printf("[SUCCESS]>File %s moved to %s\n", params[0], params[1]);
					free(params[0]);
					free(params[1]);
					free(params);
					break;
				default:
					break;
			}
			break;
		case MSG_BADRQST:
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
	int fd;

	fd = open(upload->path, O_RDONLY);

	while ((rd = read(fd, data, BSIZE)) > 0)
		ruspSend(upload->conn, data, rd);

	ruspClose(upload->conn);

	close(fd);

	return NULL;
}

static void *rcvFile(void *arg) {
	DataTransfer *download = (DataTransfer *) arg;
	char data[BSIZE];
	ssize_t rcvd, wr;
	int fd;

	fd = open(download->path, O_RDWR, O_CREAT|O_TRUNC);

	while ((rcvd = ruspReceive(download->conn, data, BSIZE)) > 0) {
		errno = 0;
		if ((wr = write(fd, data, rcvd)) != rcvd)
			ERREXIT("Cannot write to file %s: %s", download->path, strerror(errno));
	}

	ruspClose(download->conn);

	close(fd);

	return NULL;
}

static void parseArguments(int argc, char **argv) {
	int opt;

	while ((opt = getopt(argc, argv, "vhp:l:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       FTP client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       FTP client based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s [address] (-p port) (-r repo) (-l loss) (-d)\n", argv[0]);
				printf("@Opts:      -p port: FTP server port number. Default (%d) if not specified.\n", PORT);
				printf("            -r repo: FTP client repository. Default (%s) if not specified.\n", REPO);
				printf("            -l loss: Uniform probability of segments loss. Default (%F) if not specified.\n", LOSS);
				printf("            -d:      Debug mode. Default (%d) if not specified\n\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				memcpy(repo, optarg, strlen(optarg));
				break;
			case 'l':
				drop = strtod(optarg, NULL);
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

	if (optind + 1 != argc)
		ERREXIT("@Usage: %s [address] (-p port) (-r repo) (-l loss) (-d)\n", argv[0]);

	memcpy(address, argv[optind], strlen(argv[optind]));

	ruspSetAttr(RUSP_ATTR_DROPR, &drop);

	ruspSetAttr(RUSP_ATTR_DEBUG, &dbg);
}
