#ifndef FILEUTIL_H_
#define FILEUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "stringutil.h"
#include "macroutil.h"

#define DIR_PERM 	(S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)
#define FILE_PERM	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)

int openFile(const char *path, int flags);

void closeFile(const int fd);

void generateSampleFile(const int fd, const long size);

long long getFileSize(const int fd);

int isEqualFile(const int fdone, const int fdtwo);

int exploreDirectory(const char *dirpath, char ***list, int *numItems);

int isFile(const char *filepath);

int mkFile(const char *filepath, const char *data);

int rmFile(const char *filepath);

int cpFile(const char *srcFilepath, const char *dstFilepath);

int mvFile(const char *srcFilepath, const char *dstFilepath);

int fileSerialization(const char *filepath, char **serializedFile);

int isDirectory(const char *dirpath);

int mkDirectory(const char *dirpath);

int rmDirectory(const char *dirpath);

int cpDirectory(const char *srcDirpath, const char *dstDirpath);

int mvDirectory(const char *srcDirpath, const char *dstDirpath);

int changeRoot(const char *rootpath);

char *getCwd(void);

int changeCwd(const char *newpath);

char *getFilename(const char *path);

char *concatPath(const char *path, const char *subpath);

#endif /* FILEUTIL_H_ */
