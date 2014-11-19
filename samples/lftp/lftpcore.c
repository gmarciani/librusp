#include "lftpcore.h"

/* GLOBAL VARIABLES */

int LFTP_DEBUG = 0;

/* MESSAGE */

static char STR_TYP[LFTP_TYP][10] = {"REQ", "SUC", "ERR"};

static char STR_ACT[LFTP_ACT][6] = {"GTCWD", "CHDIR", "LSDIR", "MKDIR", "RMDIR", "CPDIR", "MVDIR", "RETRF", "STORF", "RMFIL", "CPFIL", "MVFIL"};

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
	char hdrf[LFTP_HDRF][3];

	memcpy(hdrf[0], smsg, sizeof(char) * 2);
	hdrf[0][2] = '\0';

	memcpy(hdrf[1], smsg + 2, sizeof(char) * 2);
	hdrf[1][2] = '\0';

	msg->header.type = atoi(hdrf[0]);

	msg->header.action = atoi(hdrf[1]);

	sprintf(msg->body, "%s", smsg + LFTP_HEAD);
}

void messageToString(const Message msg, char *strmsg) {
	sprintf(strmsg, "%s %s %s", STR_TYP[msg.header.type], STR_ACT[msg.header.action], msg.body);
}

/* MESSAGE I/O */

ssize_t sendMessage(const ConnectionId conn, const Message msg) {
	char smsg[LFTP_MSGS];
	ssize_t snd;
	size_t msgsize;

	msgsize = serializeMessage(msg, smsg);

	snd = ruspSend(conn, smsg, msgsize);

	DBGFUNC(LFTP_DEBUG, printOutMessage(conn, msg));

	return snd;
}

ssize_t receiveMessage(const ConnectionId conn, Message *msg) {
	char smsg[LFTP_MSGS];
	ssize_t rcvd;

	if ((rcvd = ruspReceive(conn, smsg, LFTP_MSGS)) > 0) {

		smsg[rcvd] = '\0';

		deserializeMessage(smsg, msg);

		DBGFUNC(LFTP_DEBUG, printInMessage(conn, *msg));
	}

	return rcvd;
}

void printOutMessage(const ConnectionId conn, const Message msg) {
	char time[TIME_STR], straddr[ADDRIPV4_STR], strmsg[LFTP_STR_MSG];
	struct sockaddr_in addr;

	getTime(time);

	ruspPeer(conn, &addr);

	addressToString(addr, straddr);

	messageToString(msg, strmsg);

	printf("[MSG ->] %s dst: %s %s\n", time, straddr, strmsg);
}

void printInMessage(const ConnectionId conn, const Message msg) {
	char time[TIME_STR], straddr[ADDRIPV4_STR], strmsg[LFTP_STR_MSG];
	struct sockaddr_in addr;

	getTime(time);

	ruspPeer(conn, &addr);

	addressToString(addr, straddr);

	messageToString(msg, strmsg);

	printf("[<- MSG] %s src: %s %s\n", time, straddr, strmsg);
}

/* MENU */

static char *MENU[LFTP_MENU_CHOICES] = {"Get CWD", "Change CWD", "List Directory", "New Directory", "Remove Directory", "Copy Directory", "Move Directory", "Download File",
								   "Upload File", "Remove File", "Copy File", "Move File", "Exit"};

int runMenu(Session *session, Message *msg) {
	int choice;
	char *inputs[2];
	int i;

	do {
		printf("\n-------\nMENU\n-------\n");
		for (i = 1; i <= LFTP_MENU_CHOICES; i++)
			printf("%d\t%s\n", i, MENU[i - 1]);
		inputs[0] = getUserInput("[Your Action]>");
		choice = atoi(inputs[0]);
		free(inputs[0]);
	} while ((choice < 1) | (choice > LFTP_MENU_CHOICES));

	msg->header.type = LFTP_REQUEST;

	switch (choice) {
		case LFTP_MENU_EXIT:
			return LFTP_MENU_EXIT;
		case LFTP_MENU_GTCWD:
			msg->header.action = LFTP_GTCWD;
			msg->body[0] = '\0';
			break;
		case LFTP_MENU_CHDIR:
			msg->header.action = LFTP_CHDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_LSDIR:
			msg->header.action = LFTP_LSDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_MKDIR:
			msg->header.action = LFTP_MKDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_RMDIR:
			msg->header.action = LFTP_RMDIR;
			inputs[0] = getUserInput("[Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_CPDIR:
			msg->header.action = LFTP_CPDIR;
			inputs[0] = getUserInput("[Src Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst Directory (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], LFTP_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case LFTP_MENU_MVDIR:
			msg->header.action = LFTP_MVDIR;
			inputs[0] = getUserInput("[Src Directory (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst Directory (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], LFTP_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case LFTP_MENU_DWFILE:
			msg->header.action = LFTP_RETRF;
			inputs[0] = getUserInput("[File (empty to abort)>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_UPFILE:
			msg->header.action = LFTP_STORF;
			while(1) {
				inputs[0] = getUserInput("[File (empty to abort)]>");
				if (strlen(inputs[0]) == 0) {
					free(inputs[0]);
					return LFTP_MENU_ERROR;
				}
				if (!isFile(inputs[0]))
					free(inputs[0]);
				else
					break;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_RMFILE:
			msg->header.action = LFTP_RMFIL;
			inputs[0] = getUserInput("[File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s", inputs[0]);
			free(inputs[0]);
			break;
		case LFTP_MENU_CPFILE:
			msg->header.action = LFTP_CPFIL;
			inputs[0] = getUserInput("[Src File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst File (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], LFTP_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case LFTP_MENU_MVFILE:
			msg->header.action = LFTP_MVFIL;
			inputs[0] = getUserInput("[Src File (empty to abort)]>");
			if (strlen(inputs[0]) == 0) {
				free(inputs[0]);
				return LFTP_MENU_ERROR;
			}
			inputs[1] = getUserInput("[Dst File (empty to abort)]>");
			if (strlen(inputs[1]) == 0) {
				free(inputs[0]);
				free(inputs[1]);
				return LFTP_MENU_ERROR;
			}
			sprintf(msg->body, "%s%s%s", inputs[0], LFTP_PDELIM, inputs[1]);
			free(inputs[0]);
			free(inputs[1]);
			break;
		default:
			break;
	}

	return choice;
}
