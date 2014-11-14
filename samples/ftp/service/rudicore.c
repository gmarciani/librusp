#include "rudicore.h"

static int MSG_RESOLUTION = 0;

static char *statusAsString[] = {"REQUEST", "SUCCESS", "BAD-REQUEST"};

static char *actionAsString[] = {"GTCWD", "CHDIR", "LSDIR", "MKDIR", "RMDIR", "CPDIR", "MVDIR", "OPFILE", "DWFILE", "UPFILE", "RMFILE", "CPFILE", "MVFILE", "ERROR"};


/* MESSAGE */

void rudiMessageDeserialization(const char *smsg, message_t *msg) {
	char **fields = NULL;
	int expfields = 4;
	int i;

	fields = splitStringNByDelimiter(smsg, MSG_FIELD_DELIM, expfields);

	rudiParseMessage(fields[0], fields[1], fields[2], fields[3], msg);	

	for (i = 0; i < expfields; i++)
		free(fields[i]);

	free(fields);
}

char *rudiMessageSerialization(const message_t msg) {	
	char *status = rudiStatusAsString(msg.header.status);
	char *action = rudiActionAsString(msg.header.action);
	char *smsg = NULL;
	size_t smsgSize;

	smsgSize = strlen(status) + strlen(action) + strlen(msg.object) + strlen(msg.body) + (strlen(MSG_FIELD_DELIM) * 3) + 1;

	if (!(smsg = malloc(sizeof(char) * smsgSize))) {
		fprintf(stderr, "Error in serialized message allocation: %s %s %s %s.\n", status, action, msg.object, msg.body);
		exit(EXIT_FAILURE);
	}

	sprintf(smsg, "%s%s%s%s%s%s%s", status, MSG_FIELD_DELIM, action, MSG_FIELD_DELIM, msg.object, MSG_FIELD_DELIM, msg.body);

	return smsg;
}

void rudiParseMessage(const char *status, const char *action, const char *object, const char *body, message_t *msg) {
	status_t msgStatus = 0;
	action_t msgAction = 0;

	if (strcmp(status, rudiStatusAsString(REQUEST)) == 0) {
		msgStatus = REQUEST;
	} else if (strcmp(status, rudiStatusAsString(SUCCESS)) == 0)  {
		msgStatus = SUCCESS;
	} else if (strcmp(status, rudiStatusAsString(BADREQUEST)) == 0) {
		msgStatus = BADREQUEST;
	} else {
		fprintf(stderr, "Error in message parsing: unknown status.\n");
		exit(EXIT_FAILURE);
	}

	if (strcmp(action, rudiActionAsString(GTCWD)) == 0) {
		msgAction = GTCWD;
	} else if (strcmp(action, rudiActionAsString(CHDIR)) == 0) {
		msgAction = CHDIR;
	} else if (strcmp(action, rudiActionAsString(LSDIR)) == 0) {
		msgAction = LSDIR;
	} else if (strcmp(action, rudiActionAsString(MKDIR)) == 0) {
		msgAction = MKDIR;
	} else if (strcmp(action, rudiActionAsString(RMDIR)) == 0) {
		msgAction = RMDIR;
	} else if (strcmp(action, rudiActionAsString(CPDIR)) == 0) {
		msgAction = CPDIR;
	} else if (strcmp(action, rudiActionAsString(MVDIR)) == 0) {
		msgAction = MVDIR;
	} else if (strcmp(action, rudiActionAsString(OPFILE)) == 0) {
		msgAction = OPFILE;
	} else if (strcmp(action, rudiActionAsString(DWFILE)) == 0) {
		msgAction = DWFILE;
	} else if (strcmp(action, rudiActionAsString(UPFILE)) == 0) {
		msgAction = UPFILE;
	} else if (strcmp(action, rudiActionAsString(RMFILE)) == 0) {
		msgAction = RMFILE;
	} else if (strcmp(action, rudiActionAsString(CPFILE)) == 0) {
		msgAction = CPFILE;
	} else if (strcmp(action, rudiActionAsString(MVFILE)) == 0) {
		msgAction = MVFILE;
	} else if (strcmp(action, rudiActionAsString(ERROR)) == 0) {
		msgAction = ERROR;
	} else {
		fprintf(stderr, "Error in message parsing: unknown action.\n");
		exit(EXIT_FAILURE);
	}

	rudiCreateMessage(msgStatus, msgAction, object, body, msg);		
}

void rudiCreateMessage(const status_t status, const action_t action, const char *object, const char *body, message_t *msg) {
	msg->header.status = status;
	msg->header.action = action;

	if (!object)
		msg->object = stringDuplication("");
	else
		msg->object = stringDuplication(object);

	if (!body)
		msg->body = stringDuplication("");
	else
		msg->body = stringDuplication(body);	
}

void rudiFreeMessage(message_t *msg) {
	if (msg->object)
		free(msg->object);
	if (msg->body)
		free(msg->body);
}


/* COMMUNICATION */

void rudiSendMessage(ConnectionId conn, const message_t msg) {
	char *smsg = rudiMessageSerialization(msg);
	rudpSend(conn, smsg);
	if (MSG_RESOLUTION)
		rudiPrintOutMessage(conn.peer, msg);
	free(smsg);
}

void rudiReceiveMessage(ConnectionId conn, message_t *msg) {
	char *smsg = rudpReceive(conn);
	rudiMessageDeserialization(smsg, msg);
	if (MSG_RESOLUTION)
		rudiPrintInMessage(conn.peer, *msg);
	free(smsg);
}
