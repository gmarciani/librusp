#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "rusp.h"
#include "lftpcore.h"

#define PORT 55000
#define REPO "."
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static int port;

static char repo[PATH_MAX];

static int debug;

static void *service(void *arg);

static void handleMessage(Session *session, const Message request);

static void *sndFile(void *arg);

static void *rcvFile(void *arg);

static void parseArguments(int argc, char **argv);

int main(int argc, char **argv) {
	ConnectionId lconn, ctrlconn;
	struct sockaddr_in addr;
	char straddr[ADDRIPV4_STR];

	parseArguments(argc, argv);

	lconn = ruspListen(PORT);

	ruspLocal(lconn, &addr);

	addressToString(addr, straddr);

	printf("WELCOME TO FTP SERVER\n\n");

	printf("Running on %s\n", straddr);

	while ((ctrlconn = ruspAccept(lconn))) {
		Session session;
		pthread_t tid;
		session.ctrlconn = ctrlconn;
		sprintf(session.cwd, "%s", repo);
		pthread_create(&tid, NULL, service, &session);
	}

	ruspClose(lconn);

	printf("Shutdown\n");

	return(EXIT_SUCCESS);
}

static void *service(void *arg) {
	Session *session = (Session *) arg;
	Message inmsg;

	while (receiveMessage(session->ctrlconn, &inmsg) > 0)
		handleMessage(session, inmsg);

	ruspClose(session->ctrlconn);

	return NULL;
}

static void handleMessage(Session *session, const Message request) {
	Message response;
	char paths[2][PATH_MAX];
	char **list;
	int items, i, res;
	size_t size;
	struct sockaddr_in paddr;
	char strpaddr[ADDRIPV4_STR];
	DataTransfer *transfer;
	pthread_t tid;

	switch (request.header.type) {
		case LFTP_REQUEST:
			switch (request.header.action) {
				case LFTP_GTCWD:
					response.header.type = LFTP_SUCCESS;
					response.header.action = LFTP_GTCWD;
					sprintf(response.body, "%s", session->cwd);
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_CHDIR:
					if ((res = changeDir(session->cwd, request.body)) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_CHDIR;
						sprintf(response.body, "Cannot change to directory %s/%s: %s", session->cwd, request.body, strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_CHDIR;
						sprintf(response.body, "%s", session->cwd);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_LSDIR:
					list = NULL;
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if ((res = exploreDirectory(paths[0], &list, &items)) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_LSDIR;
						sprintf(response.body, "Cannot list directory %s: %s", paths[0], strerror(res));
					} else {
						char *content;
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_LSDIR;
						content = arraySerialization(list, items, LFTP_PDELIM);
						sprintf(response.body, "%s;%s", session->cwd, content);
						free(content);
					}
					sendMessage(session->ctrlconn, response);
					for (i = 0; i < items; i++)
						free(list[i]);
					if (list)
						free(list);
					break;
				case LFTP_MKDIR:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if ((res = mkDirectory(paths[0])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_MKDIR;
						sprintf(response.body, "Cannot create directory %s: %s", paths[0], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_MKDIR;
						sprintf(response.body, "%s", paths[0]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_RMDIR:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if ((res = rmDirectory(paths[0])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_RMDIR;
						sprintf(response.body, "Cannot remove directory %s: %s", paths[0], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_RMDIR;
						sprintf(response.body, "%s", paths[0]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_CPDIR:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_CPDIR;
						sprintf(response.body, "Cannot copy directory %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%s/%.*s", session->cwd, i, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if ((res = cpDirectory(paths[0], paths[1])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_CPDIR;
						sprintf(response.body, "Cannot copy directory %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_CPDIR;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_MVDIR:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_MVDIR;
						sprintf(response.body, "Cannot move directory %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%s/%.*s", session->cwd, i, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if ((res = mvDirectory(paths[0], paths[1])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_MVDIR;
						sprintf(response.body, "Cannot move directory %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_MVDIR;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_RETRF:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (!isFile(paths[0])) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_RETRF;
						sprintf(response.body, "Cannot download file %s: file not present", paths[0]);
						sendMessage(session->ctrlconn, response);
						break;
					}
					response.header.type = LFTP_SUCCESS;
					response.header.action = LFTP_RETRF;
					sprintf(response.body, "%s", paths[0]);
					sendMessage(session->ctrlconn, response);
					transfer = malloc(sizeof(DataTransfer));
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					transfer->fd = open(paths[0], O_RDONLY);
					pthread_create(&tid, NULL, sndFile, transfer);
					break;
				case LFTP_STORF:
					response.header.type = LFTP_SUCCESS;
					response.header.action = LFTP_STORF;
					sprintf(response.body, "%s", request.body);
					sendMessage(session->ctrlconn, response);
					getFilename(request.body, paths[0]);
					transfer = malloc(sizeof(DataTransfer));
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					sprintf(paths[1], "%s/%s", session->cwd, paths[0]);
					transfer->fd = open(paths[1], O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					pthread_create(&tid, NULL, rcvFile, transfer);
					break;
				case LFTP_RMFIL:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if ((res = rmFile(paths[0])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_RMFIL;
						sprintf(response.body, "Cannot remove file %s: %s", paths[0], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_RMFIL;
						sprintf(response.body, "%s", paths[0]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_CPFIL:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_CPFIL;
						sprintf(response.body, "Cannot copy file %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%s/%.*s", session->cwd, i, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if ((res = cpFile(paths[0], paths[1])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_CPFIL;
						sprintf(response.body, "Cannot copy file %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_CPFIL;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case LFTP_MVFIL:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_MVFIL;
						sprintf(response.body, "Cannot move file %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%s/%.*s", session->cwd, i, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if ((res = mvFile(paths[0], paths[1])) != 0) {
						response.header.type = LFTP_BADRQST;
						response.header.action = LFTP_MVFIL;
						sprintf(response.body, "Cannot move file %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = LFTP_SUCCESS;
						response.header.action = LFTP_MVFIL;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				default:
					response.header.type = LFTP_BADRQST;
					response.header.action = request.header.action;
					sprintf(response.body, "Communication error: no valid request received (%d).", request.header.action);
					sendMessage(session->ctrlconn, response);
					break;
			}
			break;
		default:
			response.header.type = LFTP_BADRQST;
			response.header.action = request.header.action;
			sprintf(response.body, "Communication error: no request received.");
			sendMessage(session->ctrlconn, response);
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
	debug = DEBUG;

	while ((opt = getopt(argc, argv, "vhp:r:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       LFTP server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       LFTP server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s (-p port) (-r repo) (-d)\n", argv[0]);
				printf("@Opts:      -p port: LFTP server port number. Default (%d) if not specified.\n", PORT);
				printf("            -r repo: LFTP server repository. Default (%s) if not specified.\n", REPO);
				printf("            -d     : Debug mode. Default (%d) if not specified.\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				memcpy(repo, optarg, strlen(optarg));
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

	LFTP_DEBUG = debug;
}
