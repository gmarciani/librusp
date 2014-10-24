#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../../util/fileutil.h"
#include "../../util/macroutil.h"

int main(int argc, char **argv) {
	int fd;

	if (argc < 3)
		ERREXIT("usage: %s [filename] [size]", argv[0]);

	printf("# Generating random file of %ld bytes: %s...", atol(argv[2]), argv[1]);

	fd = openFile(argv[1], O_RDWR|O_CREAT|O_TRUNC);

	generateSampleFile(fd, atol(argv[2]));

	assert(getFileSize(fd) == atol(argv[2]));

	closeFile(fd);

	printf("OK\n");

	exit(EXIT_SUCCESS);
}
