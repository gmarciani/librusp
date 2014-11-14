#include "rudimenu.h"


/* ACTION MENU ENTRIES */

static int ACTION_MENU_CHOICES = 15;
static char *ACTION_MENU[15] = {"Get CWD", 
								"Change CWD", 
								"List Directory", 
								"New Directory", 
								"Remove Directory", 
								"Copy Directory", 
								"Move Directory", 
								"New File",
								"Open File", 
								"Download File", 
								"Upload File", 
								"Remove File", 
								"Copy File", 
								"Move File",
								"Exit"
								};


/* MENU AND INPUTS */

menuChoice_t rudiMenu(message_t *msg) {
	menuChoice_t actc;
	action_t msgact = 0;	
	char *actinput = NULL;	
	char *msgobj = NULL;	
	char *msgbdy = NULL;		
	int i;

	do {
		printf("\n-------\nACTIONS\n-------\n");
		for (i = 0; i < ACTION_MENU_CHOICES; i++)
			printf("%d\t%s\n", i, ACTION_MENU[i]);
		actinput = getUserInput("[Your Action]>");
		actc = atoi(actinput);
		free(actinput);
	} while ((actc < 0) | (actc >= ACTION_MENU_CHOICES));

	switch (actc) {
		char *args[2] = {NULL, NULL};
		char *srcFilename, *dstFilename = NULL;
		case MENU_EXIT:
			return MENU_EXIT;
		case MENU_CHDIR:
			msgact = CHDIR;
			msgobj = getUserInput("[Directory]>");
			break;
		case MENU_LSDIR:
			msgact = LSDIR;
			msgobj = getUserInput("[Directory]>");
			break;
		case MENU_MKDIR:
			msgact = MKDIR;
			msgobj = getUserInput("[Directory]>");
			break;
		case MENU_RMDIR:
			msgact = RMDIR;
			msgobj = getUserInput("[Directory]>");
			break;
		case MENU_CPDIR:			
			msgact = CPDIR;
			args[0] = getUserInput("[Src Directory]>");
			args[1] = getUserInput("[Dst Directory]>");
			msgobj = arraySerialization(args, 2, OBJECT_FIELDS_DELIMITER);
			free(args[0]);
			free(args[1]);
			break;
		case MENU_MVDIR:
			msgact = MVDIR;
			args[0] = getUserInput("[Src Directory]>");
			args[1] = getUserInput("[Dst Directory]>");
			msgobj = arraySerialization(args, 2, OBJECT_FIELDS_DELIMITER);
			free(args[0]);
			free(args[1]);
			break;
		case MENU_MKFILE:
			msgact = UPFILE;
			msgobj = getUserInput("[File]>");
			msgbdy = getUserInput("[Data]>");
			break;
		case MENU_OPFILE:
			msgact = OPFILE;
			msgobj = getUserInput("[File]>");
			break;
		case MENU_DWFILE:
			msgact = DWFILE;
			msgobj = getUserInput("[File]>");
			break;
		case MENU_UPFILE:
			msgact = UPFILE;
			while (1) {
				srcFilename = getUserInput("[Src File]>");
				if (isFile(srcFilename))
					break;
				free(srcFilename);
			}
			if (fileSerialization(srcFilename, &msgbdy) != 0) {
				fprintf(stderr, "Error in file serialization.\n");
				exit(EXIT_FAILURE);
			}
			dstFilename = getUserInput("[Dst File (empty-no-change)]>");
			if (strlen(dstFilename) == 0)
				msgobj = getFilename(srcFilename);
			else
				msgobj = stringDuplication(dstFilename);
			free(srcFilename);
			free(dstFilename);
			break;
		case MENU_RMFILE:
			msgact = RMFILE;
			msgobj = getUserInput("[File]>");
			break;
		case MENU_CPFILE:
			msgact = CPFILE;
			args[0] = getUserInput("[Src File]>");
			args[1] = getUserInput("[Dst File]>");
			msgobj = arraySerialization(args, 2, OBJECT_FIELDS_DELIMITER);
			free(args[0]);
			free(args[1]);
			break;
		case MENU_MVFILE:
			msgact = MVFILE;
			args[0] = getUserInput("[Src File]>");
			args[1] = getUserInput("[Dst File]>");
			msgobj = arraySerialization(args, 2, OBJECT_FIELDS_DELIMITER);
			free(args[0]);
			free(args[1]);
			break;
		default:					
			break;
	}	

	rudiCreateMessage(REQUEST, msgact, msgobj, msgbdy, msg);

	if (msgobj)
		free(msgobj);
	if (msgbdy)
		free(msgbdy);

	return actc;
}
