#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "rusp.h"
#include "ftpcore.h"

#define PORT 55000
#define DEBUG 0
#define BSIZE RUSP_WNDS * RUSP_PLDS

static int port;

static int debug;

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
		Message inmsg;
		switch (fork()) {
			case 0:
				//ruspClose(lconn);
				session.ctrlconn = ctrlconn;
				while (receiveMessage(session.ctrlconn, &inmsg) > 0)
					handleMessage(&session, inmsg);
				ruspClose(session.ctrlconn);
				exit(EXIT_SUCCESS);
				break;
			default:
				break;
		}

	}

	ruspClose(lconn);

	printf("Shutdown\n");

	return(EXIT_SUCCESS);
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
		case MSG_REQUEST:
			switch (request.header.action) {
				case MSG_GTCWD:
					if ((res = getCwd(response.body)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_GTCWD;
						sprintf(response.body, "Cannot get CWD: %s", strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_GTCWD;
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_CHDIR:
					if ((res = chDirectory(request.body)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CHDIR;
						sprintf(response.body, "Cannot change to directory %s: %s", request.body, strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_CHDIR;
						getCwd(response.body);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_LSDIR:
					list = NULL;
					if ((res = exploreDirectory(request.body, &list, &items)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_LSDIR;
						sprintf(response.body, "Cannot list directory %s: %s", request.body, strerror(res));
					} else {
						char *content;
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_LSDIR;
						content = arraySerialization(list, items, MSG_PDELIM);
						getCwd(response.body);
						strcat(response.body, ";");
						strcat(response.body, content);
						free(content);
					}
					sendMessage(session->ctrlconn, response);
					for (i = 0; i < items; i++)
						free(list[i]);
					if (list)
						free(list);
					break;
				case MSG_MKDIR:
					if ((res = mkDirectory(request.body)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MKDIR;
						sprintf(response.body, "Cannot create directory %s: %s", request.body, strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_MKDIR;
						sprintf(response.body, "%s", request.body);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_RMDIR:
					if ((res = rmDirectory(request.body)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RMDIR;
						sprintf(response.body, "Cannot remove directory %s: %s", request.body, strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_RMDIR;
						sprintf(response.body, "%s", request.body);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_CPDIR:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPDIR;
						sprintf(response.body, "Cannot copy directory %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%.*s", i, request.body);
					sprintf(paths[1], "%s", request.body + i + 1);
					if ((res = cpDirectory(paths[0], paths[1])) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPDIR;
						sprintf(response.body, "Cannot copy directory %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_CPDIR;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_MVDIR:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVDIR;
						sprintf(response.body, "Cannot move directory %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%.*s", i, request.body);
					sprintf(paths[1], "%s", request.body + i + 1);
					if ((res = mvDirectory(paths[0], paths[1])) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVDIR;
						sprintf(response.body, "Cannot move directory %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_MVDIR;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_RETRF:
					if (!isFile(request.body)) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RETRF;
						sprintf(response.body, "Cannot download file %s: file not present", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					response.header.type = MSG_SUCCESS;
					response.header.action = MSG_RETRF;
					sprintf(response.body, "%s", request.body);
					sendMessage(session->ctrlconn, response);
					transfer = malloc(sizeof(DataTransfer));
					transfer->fd = open(request.body, O_RDONLY);
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					pthread_create(&tid, NULL, sndFile, transfer);
					break;
				case MSG_STORF:
					response.header.type = MSG_SUCCESS;
					response.header.action = MSG_STORF;
					sprintf(response.body, "%s", request.body);
					sendMessage(session->ctrlconn, response);
					getFilename(request.body, paths[0]);
					transfer = malloc(sizeof(DataTransfer));
					transfer->fd = open(paths[0], O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					pthread_create(&tid, NULL, rcvFile, transfer);
					break;
				case MSG_RMFIL:
					if ((res = rmFile(request.body)) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RMFIL;
						sprintf(response.body, "Cannot remove file %s: %s", request.body, strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_RMFIL;
						sprintf(response.body, "%s", request.body);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_CPFIL:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPFIL;
						sprintf(response.body, "Cannot copy file %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%.*s", i, request.body);
					sprintf(paths[1], "%s", request.body + i + 1);
					if ((res = cpFile(paths[0], paths[1])) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPFIL;
						sprintf(response.body, "Cannot copy file %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_CPFIL;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_MVFIL:
					size = strlen(request.body);
					i = 0;
					for (i = 0; i < size; i++)
						if (request.body[i] == ';')
							break;
					if (i == size) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVFIL;
						sprintf(response.body, "Cannot move file %s: no destination specified", request.body);
						sendMessage(session->ctrlconn, response);
						break;
					}
					sprintf(paths[0], "%.*s", i, request.body);
					sprintf(paths[1], "%s", request.body + i + 1);
					if ((res = mvFile(paths[0], paths[1])) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVFIL;
						sprintf(response.body, "Cannot move file %s to %s: %s", paths[0], paths[1], strerror(res));
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_MVFIL;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				default:
					response.header.type = MSG_BADRQST;
					response.header.action = request.header.action;
					sprintf(response.body, "Communication error: no valid request received (%d).", request.header.action);
					sendMessage(session->ctrlconn, response);
					break;
			}
			break;
		default:
			response.header.type = MSG_BADRQST;
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
	debug = DEBUG;

	while ((opt = getopt(argc, argv, "vhp:r:d")) != -1) {
		switch (opt) {
			case 'v':
				printf("@App:       FTP server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				exit(EXIT_SUCCESS);
			case 'h':
				printf("@App:       FTP server based on RUSP1.0.\n");
				printf("@Version:   1.0\n");
				printf("@Author:    Giacomo Marciani\n");
				printf("@Website:   http://gmarciani.com\n");
				printf("@Email:     giacomo.marciani@gmail.com\n\n");
				printf("@Usage:     %s (-p port) (-r repo) (-d)\n", argv[0]);
				printf("@Opts:      -p port: FTP server port number. Default (%d) if not specified.\n", PORT);
				printf("            -r repo: FTP server repository. Default CWD if not specified.\n");
				printf("            -d     : Debug mode. Default (%d) if not specified.\n", DEBUG);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				errno = 0;
				if (chdir(optarg) != 0)
					ERREXIT("Bad repository %s: %s", optarg, strerror(errno));
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

	FTP_DEBUG = debug;
}
