#include "filemng.h"


// CONTENT EXPLORATION

int exploreDirectory(const char *dirpath, char ***list, int *numItems) {
	struct dirent **direntList;
	char *path;
	int i;

	errno = 0;
    if ((*numItems = scandir(dirpath, &direntList, NULL, alphasort)) == -1)
		return errno;

	if (!((*list) = malloc(sizeof(char *) * (*numItems)))) {
		fprintf(stderr, "Error in list allocation for directory exploration: %s.\n", dirpath);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < *numItems; i++) {
		path = concatPath(dirpath, direntList[i]->d_name);
		if (isDirectory(path)) 
			(*list)[i] = stringConcatenation(direntList[i]->d_name, "/");
		else		
			(*list)[i] = stringDuplication(direntList[i]->d_name);
		free(path);
	}

	for (i = 0; i < *numItems; i++)
		free(direntList[i]);
	free(direntList);

	return 0;
}


// FILES

int isFile(const char *filepath) {
	struct stat info;

	errno = 0;
	if (stat(filepath, &info) == -1) {
		if (errno == ENOENT)
			return 0;
		fprintf(stderr, "Error in stat for file existence check: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return S_ISREG(info.st_mode);
}

int mkFile(const char *filepath, const char *data) {
	int fd;

	errno = 0;
	if ((fd = open(filepath, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, FILE_PERM)) == -1)
		return errno;

	errno = 0;
	if (write(fd, data, strlen(data)) == -1)
		return errno;

	errno = 0;
	if (close(fd) == -1)
		return errno;

	return 0;
}

int rmFile(const char *filepath) {
	errno = 0;
	if (unlink(filepath) == -1)
		return errno;

	return 0;
}

int cpFile(const char *srcFilepath, const char *dstFilepath) {
	int srcFd, dstFd;	
	ssize_t numRead;
	size_t buffSize = 1024;
	char buffer[buffSize];

	errno = 0;
	if ((srcFd = open(srcFilepath, O_RDONLY)) == -1)
		return errno;

	errno = 0;
	if ((dstFd = open(dstFilepath, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, FILE_PERM)) == -1)
		return errno;

	while ((numRead = read(srcFd, buffer, buffSize)) > 0) {
		if (write(dstFd, buffer, numRead) != numRead) {
			fprintf(stderr, "Error in writing buffer for file copy.\n");
			break;
		}
	}			

	if (numRead == -1)
		fprintf(stderr, "Error in reading buffer for file copy.\n");

	errno = 0;
	if (close(srcFd) == -1)
		return errno;

	errno = 0;
	if (close(dstFd) == -1)
		return errno;

	return 0;
}

int mvFile(const char *srcFilepath, const char *dstFilepath) {
	int error;
	
	if ((error = cpFile(srcFilepath, dstFilepath)) != 0)
		return error;

	if ((error = rmFile(srcFilepath)) != 0)
		return error;

	return 0;
}

int fileSerialization(const char *filepath, char **serializedFile) {
	int fd;
	struct stat info;
	size_t toRead;
	ssize_t numRead;

	errno = 0;
	if ((fd = open(filepath, O_RDONLY)) == -1)
		return errno;

	if (fstat(fd, &info) == -1) {
		fprintf(stderr, "Error in fstat for file serialization.\n");
		exit(EXIT_FAILURE);
	}

	toRead = (size_t) info.st_size;

	if (!(*serializedFile = malloc(sizeof(char) * (toRead + 1)))) {
		fprintf(stderr, "Error in string allocation for file serialization.\n");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	if ((numRead = read(fd, (*serializedFile), toRead)) == -1)
		return errno;
	
	(*serializedFile)[numRead] = '\0';	
	
	errno = 0;
	if (close(fd) == -1)
		return errno;

	return 0;
}


// DIRECTORIES

int isDirectory(const char *dirpath) {
	struct stat info;

	errno = 0;
	if (stat(dirpath, &info) == -1) {
		if (errno == ENOENT)
			return 0;
		fprintf(stderr, "Error in stat for directory existence check: %s %s.\n", dirpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return S_ISDIR(info.st_mode);
}

int mkDirectory(const char *dirname) {
	errno = 0;	
	if (mkdir(dirname, DIR_PERM) == -1)
		return errno;

	return 0;
}

int rmDirectory(const char *dirpath) {
	char **items = NULL;
	char *path = NULL;
	int numItems;
	int error;
	int i;	

	if ((error = exploreDirectory(dirpath, &items, &numItems)) != 0)
		return error;

	for (i = 0; i < numItems; i++) {
		if ((strcmp(items[i], "./") == 0) | (strcmp(items[i], "../") == 0))
			continue;
		path = concatPath(dirpath, items[i]);
		if (isFile(path)) {
			if ((error = rmFile(path)) != 0) {
				free(path);
				return error;
			}
		} else if (isDirectory(path)) {
			if ((error = rmDirectory(path)) != 0) {
				free(path);
				return error;
			}
		}	
		free(path);		
	}

	for (i = 0; i < numItems; i++)
		free(items[i]);

	free(items);

	errno = 0;
	if (rmdir(dirpath) == -1)
		return errno;	

	return 0;
}

int cpDirectory(const char *srcDirpath, const char *dstDirpath) {
	char **items = NULL;
	char *srcPath = NULL;
	char *dstPath = NULL;
	int numItems;
	int error;
	int i;		

	if ((error = exploreDirectory(srcDirpath, &items, &numItems)) != 0)
		return error;

	if ((error = mkDirectory(dstDirpath)) != 0)
		return error;

	for (i = 0; i < numItems; i++) {
		if ((strcmp(items[i], "./") == 0) | (strcmp(items[i], "../") == 0))
			continue;
		srcPath = concatPath(srcDirpath, items[i]);
		dstPath = concatPath(dstDirpath, items[i]);
		if (isDirectory(srcPath)) {
			if ((error = cpDirectory(srcPath, dstPath)) != 0) {
				free(srcPath);
				free(dstPath);
				return error;
			}
		} else if (isFile(srcPath)) {
			if ((error = cpFile(srcPath, dstPath)) != 0) {
				free(srcPath);
				free(dstPath);
				return error;
			}				
		}	
		free(srcPath);
		free(dstPath);						
	}	

	for (i = 0; i < numItems; i++)
		free(items[i]);

	free(items);

	return 0;
}

int mvDirectory(const char *dirpath, const char *newDirpath) {
	int error;
	
	if ((error = cpDirectory(dirpath, newDirpath)) != 0)
		return error;

	if ((error = rmDirectory(dirpath)) != 0)
		return error;

	return 0;
}

int changeRoot(const char *rootpath) {
	errno = 0;
	if (chroot(rootpath) == -1)
		return errno;

	errno = 0;
	if (chdir("/") == -1)
		return errno;
	
	return 0;
}

char *getCwd(void) {
	char *cwdpath;

	if (!(cwdpath = malloc(sizeof(char) * PATH_MAX))) {
		fprintf(stderr, "Error in string allocation in getCwd.\n");
		exit(EXIT_FAILURE);
	}

	if (getcwd(cwdpath, PATH_MAX) == NULL)	{
		fprintf(stderr, "Error in getcwd.\n");
		exit(EXIT_FAILURE);
	}

	return cwdpath;
}

int changeCwd(const char *newpath) {
	return chdir(newpath);
}


// UTILS

char *getFilename(const char *path) {
	char *filename = NULL;
	size_t pathsize = strlen(path);
	int i;

	if (pathsize == 0)
		return "";

	for (i = pathsize - 1; i >= 0; i--) {
		if (path[i] == '/')
			break;
	}

	if (i == 0)
		filename = stringDuplication(path);
	else if (i == pathsize - 1)
		filename = stringDuplication("");
	else
		filename = stringDuplication(path + i + 1);

	return filename;
}

char *concatPath(const char *path, const char *subpath) {
	char *resultPath = NULL;
	size_t pathSize = strlen(path) + strlen(subpath) + 1;

	if (!(resultPath = malloc(sizeof(char) * (pathSize + 1)))) {
		fprintf(stderr, "Error in string allocation for path creation.\n");
		exit(EXIT_FAILURE);
	}
	
	sprintf(resultPath, "%s/%s", path, subpath);

	return resultPath;
}
