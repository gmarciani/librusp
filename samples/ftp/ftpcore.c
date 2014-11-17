#include "ftpcore.h"

/* GLOBAL VARIABLES */

int FTP_DEBUG = 0;

/* MESSAGE */

static char STR_TYP[MSG_TYP][10] = {"REQ", "SUC", "ERR"};

static char STR_ACT[MSG_ACT][6] = {"GTCWD", "CHDIR", "LSDIR", "MKDIR", "RMDIR", "CPDIR", "MVDIR", "RETRF", "STORF", "RMFIL", "CPFIL", "MVFIL"};

Message createMessage(const int type, const int action, const char *body) {
	Message msg;
	size_t size;

	msg.header.type = type;

	msg.header.action = action;

	size = strlen(body);

	if (size > 0)
		memcpy(msg.body, body, sizeof(char) * size);

	msg.body[size] = '\0';

	return msg;
}

size_t serializeMessage(const Message msg, char *smsg) {
	size_t size;

	sprintf(smsg, "%02d%02d%s", msg.header.type, msg.header.action, msg.body);

	size = strlen(smsg);

	return size;
}

void deserializeMessage(const char *smsg, Message *msg) {
	char hdrf[MSG_HDRF][3];

	memcpy(hdrf[0], smsg, sizeof(char) * 2);
	hdrf[0][2] = '\0';

	memcpy(hdrf[1], smsg + 2, sizeof(char) * 2);
	hdrf[1][2] = '\0';

	msg->header.type = atoi(hdrf[0]);

	msg->header.action = atoi(hdrf[1]);

	sprintf(msg->body, "%s", smsg + MSG_HEAD);
}

void messageToString(const Message msg, char *strmsg) {
	sprintf(strmsg, "%s %s %s", STR_TYP[msg.header.type], STR_ACT[msg.header.action], msg.body);
}

/* MESSAGE I/O */

ssize_t sendMessage(const ConnectionId conn, const Message msg) {
	char smsg[MSGSIZE];
	ssize_t snd;
	size_t msgsize;

	msgsize = serializeMessage(msg, smsg);

	snd = ruspSend(conn, smsg, msgsize);

	DBGFUNC(FTP_DEBUG, printOutMessage(conn, msg));

	return snd;
}

ssize_t receiveMessage(const ConnectionId conn, Message *msg) {
	char smsg[MSGSIZE];
	ssize_t rcvd;

	if ((rcvd = ruspReceive(conn, smsg, MSGSIZE)) > 0) {

		smsg[rcvd] = '\0';

		deserializeMessage(smsg, msg);

		DBGFUNC(FTP_DEBUG, printInMessage(conn, *msg));
	}

	return rcvd;
}

void printOutMessage(const ConnectionId conn, const Message msg) {
	char time[TIME_STR], straddr[ADDRIPV4_STR], strmsg[MSG_STR];
	struct sockaddr_in addr;

	getTime(time);

	ruspPeer(conn, &addr);

	addressToString(addr, straddr);

	messageToString(msg, strmsg);

	printf("[MSG ->] %s dst: %s %s\n", time, straddr, strmsg);
}

void printInMessage(const ConnectionId conn, const Message msg) {
	char time[TIME_STR], straddr[ADDRIPV4_STR], strmsg[MSG_STR];
	struct sockaddr_in addr;

	getTime(time);

	ruspPeer(conn, &addr);

	addressToString(addr, straddr);

	messageToString(msg, strmsg);

	printf("[<- MSG] %s src: %s %s\n", time, straddr, strmsg);
}

/* MENU */

static char *MENU[MENU_CHOICES] = {"Get CWD", "Change CWD", "List Directory", "New Directory", "Remove Directory", "Copy Directory", "Move Directory", "Download File",
								   "Upload File", "Remove File", "Copy File", "Move File", "Exit"};

int runMenu(Session *session, Message *msg) {
	int choice;
	char *inputs[2];
	int i;

	do {
		printf("\n-------\nMENU\n-------\n");
		for (i = 1; i <= MENU_CHOICES; i++)
			printf("%d\t%s\n", i, MENU[i - 1]);
		inputs[0] = getUserInput("[Your Action]>");
		choice = atoi(inputs[0]);
		free(inputs[0]);
	} while ((choice < 1) | (choice > MENU_CHOICES));

	msg->header.type = MSG_REQUEST;

	switch (choice) {
		case MENU_EXIT:
			return MENU_EXIT;
		case MENU_GTCWD:
			msg->header.action = MSG_GTCWD;
			msg->body[0] = '\0';
			break;
		case MENU_CHDIR:
			msg->header.action = MSG_CHDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_LSDIR:
			msg->header.action = MSG_LSDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_MKDIR:
			msg->header.action = MSG_MKDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_RMDIR:
			msg->header.action = MSG_RMDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_CPDIR:
			msg->header.action = MSG_CPDIR;
			inputs[0] = getUserInput("[Src Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst Directory (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], MSG_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_MVDIR:
			msg->header.action = MSG_MVDIR;
			inputs[0] = getUserInput("[Src Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst Directory (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], MSG_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_DWFILE:
			msg->header.action = MSG_RETRF;
			inputs[0] = getUserInput("[File (empty to abort)>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_UPFILE:
			msg->header.action = MSG_STORF;
			while(1) {
				inputs[0] = getUserInput("[File (empty to abort)]>");
				if (strlen(inputs[0]) == 0) {
					free(inputs[0]);
					return MENU_ERROR;
				}
				if (!isFile(inputs[0]))
					free(inputs[0]);
				else
					break;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_RMFILE:
			msg->header.action = MSG_RMFIL;
			inputs[0] = getUserInput("[File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case MENU_CPFILE:
			msg->header.action = MSG_CPFIL;
			inputs[0] = getUserInput("[Src File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst File (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], MSG_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_MVFILE:
			msg->header.action = MSG_MVFIL;
			inputs[0] = getUserInput("[Src File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst File (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], MSG_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		default:
			break;
	}

	return choice;
}
