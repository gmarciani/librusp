#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "../../util/cliutil.h"
#include "../../util/fileutil.h"
#include "../../util/timeutil.h"
#include "../../util/macroutil.h"

#define DBG_NONE 0b000
#define DBG_OPEN 0b001
#define DBG_TRAN 0b010
#define DBG_CLOS 0b100

static char *ADDRESS;

static int PORT;

static char *FILESND;

static int CONN;

static void establishConnection(void);

static void connectionDetails(void);

static void sendFile();

static void closeConnection(void);

int main(int argc, char **argv) {	
	
	if (argc < 4)
		ERREXIT("usage: %s [address] [port] [file]", argv[0]);

	ADDRESS = argv[1];

	PORT = atoi(argv[2]);

	FILESND = argv[3];

	establishConnection();

	connectionDetails();

	sendFile();

	closeConnection();

	exit(EXIT_SUCCESS);
}

static void establishConnection(void) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;

	printf("# Connecting to %s:%d...", ADDRESS, PORT);

	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;

	addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, ADDRESS, &(addr.sin_addr)) <= 0)
		ERREXIT("Error in address to-network translation: %s:%d.", ADDRESS, PORT);

	if ((CONN = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		ERREXIT("Cannot establish connection.");

	if (connect(CONN, (const struct sockaddr *)&addr, socksize) == -1)
		ERREXIT("Cannot establish connection.");

	printf("OK\n");
}

static void connectionDetails(void) {
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in aaddr, caddr;
	char aip[INET_ADDRSTRLEN], cip[INET_ADDRSTRLEN];
	int aport, cport;

	errno = 0;
	if (getsockname(CONN, (struct sockaddr *)&aaddr, &socksize) == -1)
		ERREXIT("Cannot get socket local address: %s", strerror(errno));

	if (!inet_ntop(AF_INET, &(aaddr.sin_addr), aip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	aport = (int) ntohs(aaddr.sin_port);

	errno = 0;
	if (getpeername(CONN, (struct sockaddr *)&caddr, &socksize) == -1)
		ERREXIT("Cannot get socket peer address: %s", strerror(errno));

	if (!inet_ntop(AF_INET, &(caddr.sin_addr), cip, INET_ADDRSTRLEN))
		ERREXIT("Cannot get address string representation.");

	cport = (int) ntohs(caddr.sin_port);

	printf("Connection established on: %s:%d with: %s:%d.\n", aip, aport, cip, cport);
}

static void sendFile() {
	char snddata[500];
	int fd;
	long long size, sent;
	ssize_t rd;
	struct timespec start, end;
	long double milliselaps, Kbps, KB;

	fd = openFile(FILESND, O_RDONLY);

	size = getFileSize(fd);

	printf("# Profiling file send on established connection: %s (%lld bytes)...\n", FILESND, size);

	sent = 0;

	milliselaps = 0.0;

	errno = 0;

	start = getTimestamp();

	while ((rd = read(fd, snddata, 500)) > 0) {

		send(CONN, snddata, rd, 0);

		memset(snddata, 0, sizeof(char) * 500);

		sent += rd;

		progressBar(sent, size);

		errno = 0;
	}

	if (rd == -1)
		ERREXIT("Cannot read file: %s.", strerror(errno));

	end = getTimestamp();

	milliselaps = getElapsed(start, end);

	closeFile(fd);

	printf(" OK\n");

	printf("# Stop sending on established connection...OK\n");

	KB = (long double)(size / 1000.0);

	Kbps = KB * 8.0 / (milliselaps / 1000.0);

	printf("Sent: %LFKB Time: %LFs Speed: %LFKbps\n", KB,  milliselaps / 1000.0, Kbps);
}

static void closeConnection(void) {
	printf("# Closing established connection...");

	if (close(CONN) == -1)
		ERREXIT("Cannot close established connection.");

	printf("OK\n");
}
