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
	off_t offone, offtwo;
	char cone, ctwo;

	errno = 0;
	if (lseek(fdone, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	errno = 0;
	if (lseek(fdtwo, 0, SEEK_SET) == -1)
		ERREXIT("Cannot lseek file: %s.", strerror(errno));

	while (1) {

		errno = 0;
		if ((offone = lseek(fdone, 0, SEEK_CUR)) == -1)
			ERREXIT("Cannot lseek file: %s.", strerror(errno));

		errno = 0;
		if ((offtwo = lseek(fdtwo, 0, SEEK_CUR)) == -1)
			ERREXIT("Cannot lseek file: %s.", strerror(errno));

		errno = 0;
		if ((rdone = read(fdone, &cone, sizeof(char))) == -1)
			ERREXIT("Cannot read file: %s.", strerror(errno));

		errno = 0;
		if ((rdtwo = read(fdtwo, &ctwo, sizeof(char))) == -1)
			ERREXIT("Cannot read file: %s.", strerror(errno));

		if (rdone == 0 && rdtwo == 0)
			break;

		if (rdone != rdtwo) {
			printf("Diff (size): rdone=%zu rdtwo=%zu\n", rdone, rdtwo);
			return 0;
		}

		if (cone != ctwo) {
			printf("Diff (content): cone=%c (at:%ld) ctwo=%c (at:%ld)\n", cone, offone, ctwo, offtwo);
			return 0;
		}
	}

	return 1;
}

// CONTENT EXPLORATION

int exploreDirectory(const char *path, char ***list, int *items) {
	struct dirent **direntList;
	int i;

	errno = 0;
    if ((*items = scandir(path, &direntList, NULL, alphasort)) == -1)
		return errno;

	if (!((*list) = malloc(sizeof(char *) * (*items))))
		ERREXIT("Error in list allocation for directory exploration: %s.\n", path);

	for (i = 0; i < *items; i++) {
		if (!((*list)[i] = malloc(sizeof(char) * PATH_MAX)))
			ERREXIT("Cannot allocate string for directory exploration.");
		if (isFile(path))
			sprintf((*list)[i], "%s", direntList[i]->d_name);
		else if (isDirectory(path))
			sprintf((*list)[i], "%s/", direntList[i]->d_name);
	}

	for (i = 0; i < *items; i++)
		free(direntList[i]);
	free(direntList);

	return 0;
}

// FILES

int isFile(const char *path) {
	struct stat info;

	errno = 0;
	if (stat(path, &info) == -1) {
		if (errno == ENOENT)
			return 0;
		fprintf(stderr, "Error in stat for file existence check: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return S_ISREG(info.st_mode);
}

int rmFile(const char *path) {
	errno = 0;
	if (unlink(path) == -1)
		return errno;

	return 0;
}

int cpFile(const char *srcpath, const char *dstpath) {
	int srcfd, dstfd;
	ssize_t rd;
	size_t bsize = 1024;
	char buffer[bsize];

	errno = 0;
	if ((srcfd = open(srcpath, O_RDONLY)) == -1)
		return errno;

	errno = 0;
	if ((dstfd = open(dstpath, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, FILE_PERM)) == -1)
		return errno;

	while ((rd = read(srcfd, buffer, bsize)) > 0) {
		if (write(dstfd, buffer, rd) != rd) {
			fprintf(stderr, "Error in writing buffer for file copy.\n");
			break;
		}
	}

	if (rd == -1)
		fprintf(stderr, "Error in reading buffer for file copy.\n");

	errno = 0;
	if (close(srcfd) == -1)
		return errno;

	errno = 0;
	if (close(dstfd) == -1)
		return errno;

	return 0;
}

int mvFile(const char *srcpath, const char *dstpath) {
	int error;

	if ((error = cpFile(srcpath, dstpath)) != 0)
		return error;

	if ((error = rmFile(srcpath)) != 0)
		return error;

	return 0;
}

int getFilename(const char *path, char *filename) {
	size_t pathsize;
	int i;

	pathsize = strlen(path);

	if (pathsize == 0 || path[pathsize] == '/' || strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
		return -1;

	for (i = pathsize; i >= 0; i--) {
		if (path[i] == '/')
			break;
	}

	if (i == 0)
		sprintf(filename, "%s", path);
	else
		sprintf(filename, "%s", path + i + 1);

	return 0;
}


// DIRECTORIES

int isDirectory(const char *path) {
	struct stat info;

	errno = 0;
	if (stat(path, &info) == -1) {
		if (errno == ENOENT)
			return 0;
		ERREXIT("Error in stat for directory existence check: %s %s.\n", path, strerror(errno));
	}

	return S_ISDIR(info.st_mode);
}

int mkDirectory(const char *path) {
	errno = 0;
	if (mkdir(path, DIR_PERM) == -1)
		return errno;

	return 0;
}

int rmDirectory(const char *path) {
	struct dirent **direntList;
	char tmppath[PATH_MAX];
	int items , error, i;

	errno = 0;
	if ((items = scandir(path, &direntList, NULL, alphasort)) == -1)
		return errno;

	for (i = 0; i < items; i++) {
		if (strcmp(direntList[i]->d_name, ".") == 0 || strcmp(direntList[i]->d_name, "..") == 0)
			continue;
		sprintf(tmppath, "%s/%s", path, direntList[i]->d_name);
		errno = 0;
		if (isDirectory(tmppath)) {
			if ((error = rmDirectory(tmppath)) != 0) {
				for (i = 0; i < items; i++)
					free(direntList[i]);
				free(direntList);
				return error;
			}
		} else if (isFile(tmppath)) {
			if ((error = rmFile(tmppath)) != 0) {
				for (i = 0; i < items; i++)
					free(direntList[i]);
				free(direntList);
				return error;
			}
		}
	}

	for (i = 0; i < items; i++)
		free(direntList[i]);
	free(direntList);

	errno = 0;
	if (rmdir(path) == -1)
		return errno;

	return 0;
}

int cpDirectory(const char *srcpath, const char *dstpath) {
	struct dirent **direntList;
	char srctmppath[PATH_MAX];
	char dsttmppath[PATH_MAX];
	int items, error, i;

	errno = 0;
	if (mkdir(dstpath, DIR_PERM) != 0)
		return errno;

	errno = 0;
	if ((items = scandir(srcpath, &direntList, NULL, alphasort)) == -1) {
		printf("Error in scandir(%s)\n", srcpath);
		return errno;
	}

	for (i = 0; i < items; i++) {
		if (strcmp(direntList[i]->d_name, ".") == 0 || strcmp(direntList[i]->d_name, "..") == 0)
			continue;
		sprintf(srctmppath, "%s/%s", srcpath, direntList[i]->d_name);
		sprintf(dsttmppath, "%s/%s", dstpath, direntList[i]->d_name);
		if (isDirectory(srctmppath)) {
			if ((error = cpDirectory(srctmppath, dsttmppath)) != 0) {
				for (i = 0; i < items; i++)
					free(direntList[i]);
				free(direntList);
				printf("Error in cpDirectory(%s, %s)\n", srctmppath, dsttmppath);
				return error;
			}
		} else if (isFile(srctmppath)) {
			if ((error = cpFile(srctmppath, dsttmppath)) != 0) {
				for (i = 0; i < items; i++)
					free(direntList[i]);
				free(direntList);
				printf("Error in cpFile(%s, %s)\n", srctmppath, dsttmppath);
				return error;
			}
		}
	}

	for (i = 0; i < items; i++)
		free(direntList[i]);
	free(direntList);

	return 0;
}

int mvDirectory(const char *srcpath, const char *dstpath) {
	int error;

	if ((error = cpDirectory(srcpath, dstpath)) != 0)
		return error;

	if ((error = rmDirectory(srcpath)) != 0)
		return error;

	return 0;
}

int changeRoot(const char *path) {
	errno = 0;
	if (chroot(path) == -1)
		return errno;

	errno = 0;
	if (chdir("/") == -1)
		return errno;

	return 0;
}

int getCwd(char *path) {
	errno = 0;
	if (!getcwd(path, PATH_MAX))
		return errno;

	return 0;
}

int changeDir(char *path, const char *change) {
	int result, i;
	char tmp[PATH_MAX];

	if ((strlen(change) == 0) ||
		(strcmp(change, ".") == 0) ||
		((strcmp(path, "/") == 0 || strcmp(path, ".") == 0) && strcmp(change, "..") == 0)) {
		result = 0;
	} else if (strcmp(change, "..") == 0) {
		for (i = strlen(path); i > 0; i--)
			if (path[i] == '/')
				break;
		path[i] = '\0';
		result = 0;
	} else if (change[0] != '/' && change[strlen(change)] != '/') {
		sprintf(tmp, "%s/%s", path, change);
		if (!isDirectory(tmp)) {
			result = -1;
		} else {
			sprintf(path, "%s", tmp);
			result = 0;
		}
	} else {
		result = -1;
	}

	return result;
}
