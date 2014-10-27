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

long long getFileSize(const int fd) {
	long long size;

	errno = 0;
	if ((size = lseek(fd, 0, SEEK_END)) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	errno = 0;
	if (lseek(fd, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	return size;
}

int isEqualFile(const int fdone, const int fdtwo) {
	ssize_t rdone, rdtwo;
	char cone, ctwo;

	errno = 0;
	if (lseek(fdone, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	errno = 0;
	if (lseek(fdtwo, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	while (1) {

		errno = 0;
		if ((rdone = read(fdone, &cone, sizeof(char))) == -1)
			ERREXIT("Cannot read file: %s.", strerror(errno));

		errno = 0;
		if ((rdtwo = read(fdtwo, &ctwo, sizeof(char))) == -1)
			ERREXIT("Cannot read file: %s.", strerror(errno));

		if (rdone == 0 && rdtwo == 0)
			break;

		if (rdone != rdtwo) {
			printf("Difference: rdone=%zu rdtwo=%zu\n", rdone, rdtwo);
			return 0;
		}

		if (cone != ctwo) {
			printf("Difference: cone=%c ctwo=%c\n", cone, ctwo);
			return 0;
		}
	}

	return 1;
}
