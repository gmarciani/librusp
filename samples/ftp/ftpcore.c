#include "ftpcore.h"

/* MESSAGE */

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

	memcpy(msg->body, smsg + MSG_HEAD, sizeof(char) * strlen(smsg + MSG_HEAD));
}

/* MESSAGE I/O */

ssize_t sendMessage(const ConnectionId conn, const Message msg) {
	char smsg[MSGSIZE];
	ssize_t snd;
	size_t msgsize;

	printf("Sending: %d %d %s\n", msg.header.type, msg.header.action, msg.body);

	msgsize = serializeMessage(msg, smsg);

	snd = ruspSend(conn, smsg, msgsize);

	return snd;
}

ssize_t receiveMessage(const ConnectionId conn, Message *msg) {
	char smsg[MSGSIZE];
	ssize_t rcvd;

	rcvd = ruspReceive(conn, smsg, MSGSIZE);

	smsg[rcvd] = '\0';

	deserializeMessage(smsg, msg);

	printf("Receiving: %d %d %s\n", msg->header.type, msg->header.action, msg->body);

	return rcvd;
}

/* MENU */

static char *MENU[MENU_CHOICES] = {"Get CWD", "Change CWD", "List Directory", "New Directory", "Remove Directory", "Copy Directory", "Move Directory", "Download File",
								   "Upload File", "Remove File", "Copy File", "Move File", "Exit"};

int runMenu(Session *session, Message *msg) {
	int choice;
	char *input;
	char *inputs[2];
	int i;

	do {
		printf("\n-------\nMENU\n-------\n");
		for (i = 1; i <= MENU_CHOICES; i++)
			printf("%d\t%s\n", i, MENU[i - 1]);
		input = getUserInput("[Your Action]>");
		choice = atoi(input);
		free(input);
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
			input = getUserInput("[Directory]>");
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_LSDIR:
			msg->header.action = MSG_LSDIR;
			input = getUserInput("[Directory]>");
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_MKDIR:
			msg->header.action = MSG_MKDIR;
			input = getUserInput("[Directory]>");
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_RMDIR:
			msg->header.action = MSG_RMDIR;
			input = getUserInput("[Directory]>");
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_CPDIR:
			msg->header.action = MSG_CPDIR;
			inputs[0] = getUserInput("[Src Directory]>");
			inputs[1] = getUserInput("[Dst Directory]>");
			sprintf(msg->body, "%s%s%s", inputs[0], OBJECT_FIELDS_DELIMITER, inputs[1]);
			free(input);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_MVDIR:
			msg->header.action = MSG_MVDIR;
			inputs[0] = getUserInput("[Src Directory]>");
			inputs[1] = getUserInput("[Dst Directory]>");
			sprintf(msg->body, "%s%s%s", inputs[0], OBJECT_FIELDS_DELIMITER, inputs[1]);
			free(input);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_DWFILE:
			msg->header.action = MSG_RETRF;
			input = getUserInput("[File (empty to abort)>");
			if (strlen(input) == 0) {
				free(input);
				return MENU_ERROR;
			}
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_UPFILE:
			msg->header.action = MSG_STORF;
			while(1) {
				input = getUserInput("[File (empty to abort)]>");
				if (strlen(input) == 0) {
					free(input);
					return MENU_ERROR;
				}
				if (!isFile(input))
					free(input);
				else
					break;
			}
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_RMFILE:
			msg->header.action = MSG_RMFIL;
			input = getUserInput("[File]>");
			sprintf(msg->body, "%s", input);
			free(input);
			break;
		case MENU_CPFILE:
			msg->header.action = MSG_CPFIL;
			inputs[0] = getUserInput("[Src File]>");
			inputs[1] = getUserInput("[Dst File]>");
			sprintf(msg->body, "%s%s%s", inputs[0], OBJECT_FIELDS_DELIMITER, inputs[1]);
			free(input);
			free(inputs[0]);
			free(inputs[1]);
			break;
		case MENU_MVFILE:
			msg->header.action = MSG_MVFIL;
			inputs[0] = getUserInput("[Src File]>");
			inputs[1] = getUserInput("[Dst File]>");
			sprintf(msg->body, "%s%s%s", inputs[0], OBJECT_FIELDS_DELIMITER, inputs[1]);
			free(input);
			free(inputs[0]);
			free(inputs[1]);
			break;
		default:
			break;
	}

	return choice;
}
