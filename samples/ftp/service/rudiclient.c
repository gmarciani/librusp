#include "rudiclient.h"


/* CONFIGURATION */

void rudiLoadClientConfig(void) {
	char *repopath;

	if (changeCwd(RUDI_CNFDIR) != 0) {
		fprintf(stderr, "Cannot find configuration folder: %s\n", RUDI_CNFDIR);
		exit(EXIT_FAILURE);
	}

	repopath = configGet(RUDI_CLIENT_CNF, RUDI_CLIENT_CNF_REPO, RUDI_CLIENT_DFL_REPO);

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


// RESPONSE HANDLER ENTRY-POINT

void rudiClientHandleResponse(const message_t inmsg) {
	switch (inmsg.header.status) {
		case SUCCESS:
			switch (inmsg.header.action) {
				case GTCWD:			
					rudiClientHandleResponseGTCWD(inmsg);
					break;
				case LSDIR:			
					rudiClientHandleResponseLSDIR(inmsg);
					break;
				case OPFILE:
					rudiClientHandleResponseOPFILE(inmsg);
					break;
				case DWFILE:
					rudiClientHandleResponseDWFILE(inmsg);
					break;
				default:
					rudiClientHandleResponseSuccess(inmsg.header.action, inmsg.object, inmsg.body);
					break;
			}
			break;
		case BADREQUEST:
			rudiClientHandleResponseBadRequest(inmsg.header.action, inmsg.object, inmsg.body);
			break;
		default:
			break;
	}
}


// SPECIFIC RESPONSE HANDLERS

void rudiClientHandleResponseGTCWD(const message_t inmsg) {
	printf("\n-----------------\nCURRENT DIRECTORY: %s\n-----------------\n", inmsg.body);
}

void rudiClientHandleResponseLSDIR(const message_t inmsg) {
	char **list;
	int numItems;
	int i;

	list = arrayDeserialization(inmsg.body, OBJECT_FIELDS_DELIMITER, &numItems);

	printf("\n---------\nFILE LIST: %s\n---------\n", inmsg.object);
	for (i = 0; i < numItems; i++)
		printf("%s\n", list[i]);

	for (i = 0; i < numItems; i++)
		free(list[i]);
	
	free(list);
}

void rudiClientHandleResponseOPFILE(const message_t inmsg) {
	printf("\n---------\nOPEN FILE: %s\n---------\n%s\n", inmsg.object, inmsg.body);
}

void rudiClientHandleResponseDWFILE(const message_t inmsg) {
	char *filepath;

	rudiClientHandleResponseSuccess(DWFILE, inmsg.object, "File successfully downloaded.");

	filepath = getUserInput("[File]>");

	if (mkFile(filepath, inmsg.body) == -1) {
		fprintf(stderr, "Error in file save.\n");
		exit(EXIT_FAILURE);
	} else {
		printf("File successfully saved.\n");
	}

	free(filepath);
}

void rudiClientHandleResponseSuccess(const action_t act, const char *obj, const char *descr) {
	printf("\n-------\nSUCCESS (%s:%s): %s\n-------\n", rudiActionAsString(act), obj, descr);
}

void rudiClientHandleResponseBadRequest(const action_t act, const char *obj, const char *descr) {
	if (act == ERROR) {
		printf("\n-----------\nBAD-REQUEST (%s): %s\n-----------\n", obj, descr);
	} else {
		printf("\n-----------\nBAD-REQUEST (%s:%s): %s\n-----------\n", rudiActionAsString(act), obj, descr);
	}	
}
