#include "fileutil.h"

int openFile(const char *path, int flags) {
	int fd;

	errno = 0;
	if ((fd = open(path, flags, S_IRWXU)) == -1)
		ERREXIT("Cannot open file %s: %s.", path, strerror(errno));

	return fd;
}

void closeFile(const int fd) {
	errno = 0;
	if (close(fd) == -1)
		ERREXIT("Cannot close file: %s.", strerror(errno));
}

void generateSampleFile(const int fd, const long size) {
	char c;
	long written;

	for (written = 0; written < size; written++) {

		c = 'A' + (rand() % 26);

		errno = 0;
		if (write(fd, &c, sizeof(char)) == -1)
			ERREXIT("Cannot write on file: %s.", strerror(errno));
	}
}

long getFileSize(const int fd) {
	long size;

	errno = 0;
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	errno = 0;
	if (lseek(fd, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	return size;
}

int isEqualFile(const int fdone, const int fdtwo) {
	char cone, ctwo;

	if (getFileSize(fdone) != getFileSize(fdtwo))
		return 0;

	while (read(fdone, &cone, sizeof(char)) && read(fdtwo, &ctwo, sizeof(char))) {
		if (cone != ctwo)
			return 0;
	}

	return 1;
}
