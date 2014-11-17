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

int isFile(const char *path);

int rmFile(const char *path);

int cpFile(const char *srcpath, const char *dstpath);

int mvFile(const char *srcpath, const char *dstpath);

int isDirectory(const char *path);

int mkDirectory(const char *path);

int rmDirectory(const char *path);

int cpDirectory(const char *srcpath, const char *dstpath);

int mvDirectory(const char *srcpath, const char *dstpath);

int changeRoot(const char *path);

int getCwd(char *path);

int changeDir(char *path, const char *change);

int getFilename(const char *path, char *filename);

#endif /* FILEUTIL_H_ */
