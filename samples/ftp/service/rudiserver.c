#include "rudiserver.h"


/* CONFIGURATION */

void rudiLoadServerConfig() {
	char *repopath;

	if (changeCwd(RUDI_CNFDIR) != 0) {
		fprintf(stderr, "Cannot find configuration folder: %s\n", RUDI_CNFDIR);
		exit(EXIT_FAILURE);
	}

	repopath = configGet(RUDI_SERVER_CNF, RUDI_SERVER_CNF_REPO, RUDI_SERVER_DFL_REPO);

	if (!isDirectory(repopath)) {
		if (mkDirectory(repopath) != 0) {
			fprintf(stderr, "Cannot create repo: %s\n", repopath);
			exit(EXIT_FAILURE);
		}
	}		

	if (changeRoot(repopath) != 0) {
		fprintf(stderr, "Cannot change root directory: %s\n", repopath);
		exit(EXIT_FAILURE);
	}
	
	free(repopath);
}


// REQUEST HANDLER ENTRY-POINT

void rudiServerHandleRequest(message_t inmsg, message_t *outmsg) {
	switch (inmsg.header.status) {
		case REQUEST:
			switch (inmsg.header.action) {
				case GTCWD:
					rudiServerHandleRequestGTCWD(inmsg, outmsg);
					break;
				case CHDIR:
					rudiServerHandleRequestCHDIR(inmsg, outmsg);
					break;
				case LSDIR:
					rudiServerHandleRequestLSDIR(inmsg, outmsg);
					break;
				case MKDIR:
					rudiServerHandleRequestMKDIR(inmsg, outmsg);
					break;
				case RMDIR:
					rudiServerHandleRequestRMDIR(inmsg, outmsg);
					break;
				case CPDIR:
					rudiServerHandleRequestCPDIR(inmsg, outmsg);
					break;
				case MVDIR:
					rudiServerHandleRequestMVDIR(inmsg, outmsg);
					break;
				case OPFILE:
					rudiServerHandleRequestOPFILE(inmsg, outmsg);
					break;
				case DWFILE:
					rudiServerHandleRequestDWFILE(inmsg, outmsg);
					break;
				case UPFILE:
					rudiServerHandleRequestUPFILE(inmsg, outmsg);
					break;
				case RMFILE:
					rudiServerHandleRequestRMFILE(inmsg, outmsg);
					break;
				case CPFILE:
					rudiServerHandleRequestCPFILE(inmsg, outmsg);
					break;
				case MVFILE:
					rudiServerHandleRequestMVFILE(inmsg, outmsg);
					break;
				default:					
					rudiCreateMessage(BADREQUEST, ERROR, "Communication Error", "Corrupted message received.", outmsg);
					break;
			}
			break;
		default:
			rudiCreateMessage(BADREQUEST, ERROR, "Communication Error", "Corrupted message received.", outmsg);
			break;
	}
}


// SPECIFIC REQUEST HANDLERS

void rudiServerHandleRequestGTCWD(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	msgBody = getCwd();

	rudiCreateMessage(SUCCESS, GTCWD, NULL, msgBody, outmsg);

	free(msgBody);
}

void rudiServerHandleRequestCHDIR(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	if (changeCwd(inmsg.object) == -1) {
		msgBody = stringDuplication("Cannot change cwd.");
		rudiCreateMessage(BADREQUEST, CHDIR, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("Directory successfully changed.");
		rudiCreateMessage(SUCCESS, CHDIR, inmsg.object, msgBody, outmsg);
	}

	free(msgBody);
}

void rudiServerHandleRequestLSDIR(const message_t inmsg, message_t *outmsg) {
	char **list = NULL;
	char *dir = NULL;
	char *msgBody = NULL;
	int numItems;
	int i;

	if (strlen(inmsg.object) == 0)
		dir = getCwd();
	else
		dir = stringDuplication(inmsg.object);

	if (exploreDirectory(dir, &list, &numItems) != 0) {
		msgBody = stringDuplication("Cannot list files in directory.");
		rudiCreateMessage(BADREQUEST, LSDIR, dir, msgBody, outmsg);
	} else {
		msgBody = arraySerialization(list, numItems, OBJECT_FIELDS_DELIMITER);	
		rudiCreateMessage(SUCCESS, LSDIR, dir, msgBody, outmsg);
	}

	for (i = 0; i < numItems; i++)
		free(list[i]);
	free(list);
	free(dir);
	free(msgBody);
}

void rudiServerHandleRequestMKDIR(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	if (mkDirectory(inmsg.object) != 0) {
		msgBody = stringDuplication("Cannot create directory.");
		rudiCreateMessage(BADREQUEST, MKDIR, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("Directory successfully created.");
		rudiCreateMessage(SUCCESS, MKDIR, inmsg.object, msgBody, outmsg);
	}

	free(msgBody);
}

void rudiServerHandleRequestRMDIR(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	if (rmDirectory(inmsg.object) != 0) {
		msgBody = stringDuplication("Cannot remove directory.");
		rudiCreateMessage(BADREQUEST, RMDIR, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("Directory successfully removed.");
		rudiCreateMessage(SUCCESS, RMDIR, inmsg.object, msgBody, outmsg);
	}

	free(msgBody);
}

void rudiServerHandleRequestCPDIR(const message_t inmsg, message_t *outmsg) {
	char **paths = NULL;
	char *msgBody = NULL;

	paths = splitStringNByDelimiter(inmsg.object, ";", 2);

	if (cpDirectory(paths[0], paths[1]) != 0) {
		msgBody = stringDuplication("Cannot copy directory.");
		rudiCreateMessage(BADREQUEST, CPDIR, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("Directory successfully copied.");
		rudiCreateMessage(SUCCESS, CPDIR, inmsg.object, msgBody, outmsg);
	}

	free(paths[0]);
	free(paths[1]);
	free(paths);
	free(msgBody);
}

void rudiServerHandleRequestMVDIR(const message_t inmsg, message_t *outmsg) {
	char **paths = NULL;
	char *msgBody = NULL;

	paths = splitStringNByDelimiter(inmsg.object, ";", 2);

	if (mvDirectory(paths[0], paths[1]) != 0) {
		msgBody = stringDuplication("Cannot move directory.");
		rudiCreateMessage(BADREQUEST, MVDIR, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("Directory successfully moved.");
		rudiCreateMessage(SUCCESS, MVDIR, inmsg.object, msgBody, outmsg);
	}

	free(paths[0]);
	free(paths[1]);
	free(paths);
	free(msgBody);
}

void rudiServerHandleRequestOPFILE(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;
	
	if (fileSerialization(inmsg.object, &msgBody) != 0) {
		msgBody = stringDuplication("Cannot open file.");
		rudiCreateMessage(BADREQUEST, OPFILE, inmsg.object, msgBody, outmsg);
	} else {
		rudiCreateMessage(SUCCESS, OPFILE, inmsg.object, msgBody, outmsg);
	}

	free(msgBody);
}

void rudiServerHandleRequestDWFILE(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;
	
	if (fileSerialization(inmsg.object, &msgBody) != 0) {
		msgBody = stringDuplication("Cannot download file.");
		rudiCreateMessage(BADREQUEST, DWFILE, inmsg.object, msgBody, outmsg);
	} else {
		rudiCreateMessage(SUCCESS, DWFILE, inmsg.object, msgBody, outmsg);
	}	

	free(msgBody);
}

void rudiServerHandleRequestUPFILE(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	if (mkFile(inmsg.object, inmsg.body) != 0) {
		msgBody = stringDuplication("Cannot upload file.");
		rudiCreateMessage(BADREQUEST, UPFILE, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("File successfully uploaded.");
		rudiCreateMessage(SUCCESS, UPFILE, inmsg.object, msgBody, outmsg);
	}	

	free(msgBody);
}

void rudiServerHandleRequestRMFILE(const message_t inmsg, message_t *outmsg) {
	char *msgBody = NULL;

	if (rmFile(inmsg.object) != 0) {
		msgBody = stringDuplication("Cannot remove file.");
		rudiCreateMessage(BADREQUEST, RMFILE, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("File successfully removed.");
		rudiCreateMessage(SUCCESS, RMFILE, inmsg.object, msgBody, outmsg);
	}	

	free(msgBody);
}

void rudiServerHandleRequestCPFILE(const message_t inmsg, message_t *outmsg) {
	char **paths = NULL;
	char *msgBody = NULL;

	paths = splitStringNByDelimiter(inmsg.object, ";", 2);

	if (cpFile(paths[0], paths[1]) != 0) {
		msgBody = stringDuplication("Cannot copy file.");
		rudiCreateMessage(BADREQUEST, CPFILE, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("File successfully copied.");
		rudiCreateMessage(SUCCESS, CPFILE, inmsg.object, msgBody, outmsg);
	}	

	free(paths[0]);
	free(paths[1]);
	free(paths);
	free(msgBody);
}

void rudiServerHandleRequestMVFILE(const message_t inmsg, message_t *outmsg) {
	char **paths = NULL;
	char *msgBody = NULL;

	paths = splitStringNByDelimiter(inmsg.object, ";", 2);

	if (mvFile(paths[0], paths[1]) != 0) {
		msgBody = stringDuplication("Cannot move file.");
		rudiCreateMessage(BADREQUEST, MVFILE, inmsg.object, msgBody, outmsg);
	} else {
		msgBody = stringDuplication("File successfully moved.");
		rudiCreateMessage(SUCCESS, MVFILE, inmsg.object, msgBody, outmsg);
	}	

	free(paths[0]);
	free(paths[1]);
	free(paths);
	free(msgBody);
}
