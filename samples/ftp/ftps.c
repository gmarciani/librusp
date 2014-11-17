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
#define BSIZE RUSP_WNDS * RUSP_PLDS

static int port;

static char repo[PATH_MAX];

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
		memcpy(session.cwd, repo, strlen(repo));
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
	char *param;
	int items, i;
	size_t size;
	struct sockaddr_in paddr;
	char strpaddr[ADDRIPV4_STR];
	DataTransfer *transfer;
	pthread_t tid;

	switch (request.header.type) {
		case MSG_REQUEST:
			switch (request.header.action) {
				case MSG_GTCWD:
					response.header.type = MSG_SUCCESS;
					response.header.action = MSG_GTCWD;
					sprintf(response.body, "%s", session->cwd);
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_CHDIR:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (!isDirectory(paths[0])) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CHDIR;
						sprintf(response.body, "Cannot change to directory %s", paths[0]);
					} else {
						sprintf(session->cwd, "%s", paths[0]);
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_CHDIR;
						sprintf(response.body, "%s", session->cwd);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_LSDIR:
					list = NULL;
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (exploreDirectory(paths[0], &list, &items) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_LSDIR;
						sprintf(response.body, "Cannot list directory %s", paths[0]);
					} else {
						char *body;
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_LSDIR;
						body = arraySerialization(list, items, OBJECT_FIELDS_DELIMITER);
						sprintf(response.body, "%s", body);
						free(body);
					}
					sendMessage(session->ctrlconn, response);
					for (i = 0; i < items; i++)
						free(list[i]);
					if (list)
						free(list);
					break;
				case MSG_MKDIR:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (mkDirectory(paths[0]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MKDIR;
						sprintf(response.body, "Cannot create directory %s", paths[0]);
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_MKDIR;
						sprintf(response.body, "%s", paths[0]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_RMDIR:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (rmDirectory(paths[0]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RMDIR;
						sprintf(response.body, "Cannot remove directory %s", paths[0]);
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_RMDIR;
						sprintf(response.body, "%s", paths[0]);
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
					sprintf(paths[0], "%s/%.*s", session->cwd, (int) i - 1, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if (cpDirectory(paths[0], paths[1]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPDIR;
						sprintf(response.body, "Cannot copy directory %s to %s", paths[0], paths[1]);
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
					sprintf(paths[0], "%s/%.*s", session->cwd, (int) i - 1, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if (cpDirectory(paths[0], paths[1]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVDIR;
						sprintf(response.body, "Cannot move directory %s to %s", paths[0], paths[1]);
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_MVDIR;
						sprintf(response.body, "%s;%s", paths[0], paths[1]);
					}
					sendMessage(session->ctrlconn, response);
					break;
				case MSG_RETRF:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (!isFile(paths[0])) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RETRF;
						sprintf(response.body, "Cannot download file %s: file not present", paths[0]);
						sendMessage(session->ctrlconn, response);
						break;
					}
					response.header.type = MSG_SUCCESS;
					response.header.action = MSG_RETRF;
					sprintf(response.body, "%s", paths[0]);
					sendMessage(session->ctrlconn, response);
					transfer = malloc(sizeof(DataTransfer));
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					sprintf(transfer->path, "%s", paths[0]);
					pthread_create(&tid, NULL, sndFile, &transfer);
					break;
				case MSG_STORF:
					response.header.type = MSG_SUCCESS;
					response.header.action = MSG_STORF;
					sprintf(response.body, "%s", request.body);
					sendMessage(session->ctrlconn, response);
					param = getFilename(request.body);
					transfer = malloc(sizeof(DataTransfer));
					ruspPeer(session->ctrlconn, &paddr);
					inet_ntop(AF_INET, &(paddr.sin_addr), strpaddr, INET_ADDRSTRLEN);
					transfer->conn = ruspConnect(strpaddr, port + 1);
					sprintf(transfer->path, "%s/%s", session->cwd, param);
					pthread_create(&tid, NULL, rcvFile, &transfer);
					break;
				case MSG_RMFIL:
					sprintf(paths[0], "%s/%s", session->cwd, request.body);
					if (rmFile(paths[0]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_RMFIL;
						sprintf(response.body, "Cannot remove file %s", paths[0]);
					} else {
						response.header.type = MSG_SUCCESS;
						response.header.action = MSG_RMFIL;
						sprintf(response.body, "%s", paths[0]);
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
					sprintf(paths[0], "%s/%.*s", session->cwd, (int) i - 1, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if (cpFile(paths[0], paths[1]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_CPFIL;
						sprintf(response.body, "Cannot copy file %s to %s", paths[0], paths[1]);
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
					sprintf(paths[0], "%s/%.*s", session->cwd, (int) i - 1, request.body);
					sprintf(paths[1], "%s/%s", session->cwd, request.body + i + 1);
					if (mvFile(paths[0], paths[1]) != 0) {
						response.header.type = MSG_BADRQST;
						response.header.action = MSG_MVFIL;
						sprintf(response.body, "Cannot move file %s to %s", paths[0], paths[1]);
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
	int fd;

	fd = open(upload->path, O_RDONLY);

	while ((rd = read(fd, data, BSIZE)) > 0)
		ruspSend(upload->conn, data, rd);

	ruspClose(upload->conn);

	close(fd);

	free(upload);

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

	free(download);

	return NULL;
}

static void parseArguments(int argc, char **argv) {
	int opt;

	port = PORT;
	sprintf(repo, "%s", REPO);

	while ((opt = getopt(argc, argv, "vhp:r:")) != -1) {
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
				printf("@Usage:     %s (-p port) (-r repo)\n", argv[0]);
				printf("@Opts:      -p port: FTP server port number. Default (%d) if not specified.\n", PORT);
				printf("            -r repo: FTP server repository. Default (%s) if not specified.\n", REPO);
				exit(EXIT_SUCCESS);
			case 'p':
				port = atoi(optarg);
				break;
			case 'r':
				memcpy(repo, optarg, strlen(optarg));
				break;
			case '?':
				printf("Bad option %c.\n", optopt);
				printf("-h for help.\n\n");
				exit(EXIT_FAILURE);
			default:
				break;
		}
	}
}
