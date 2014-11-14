#ifndef RUDICORE_H_
#define RUDICORE_H_

#include "rusp.h"
#include "../common/filemng.h"
#include "../common/util.h"

/* MESSAGE */
#define MSG_FIELD_DELIM "\\"
#define OBJECT_FIELDS_DELIMITER ";"

/* MESSAGE */
typedef enum status_t {
	REQUEST,
	SUCCESS,
	BADREQUEST
} status_t;

typedef enum action_t {
	GTCWD,
	CHDIR,
	LSDIR,
	MKDIR,
	RMDIR,
	CPDIR,
	MVDIR,
	OPFILE,
	DWFILE,
	UPFILE,
	RMFILE,
	CPFILE,
	MVFILE,
	ERROR
} action_t;

typedef struct msgheader_t {
	status_t status;
	action_t action;	
} msgheader_t;

typedef struct message_t {
	msgheader_t header;
	char *object;
	char *body;
} message_t;

void rudiMessageDeserialization(const char *smsg, message_t *msg);

char *rudiMessageSerialization(const message_t msg);

void rudiParseMessage(const char *status, const char *action, const char *object, const char *body, message_t *msg);

void rudiCreateMessage(const status_t status, const action_t, const char *object, const char *body, message_t *msg);

void rudiFreeMessage(message_t *msg);


/* COMMUNICATION */

void rudiSendMessage(ConnectionId conn, const message_t msg);

void rudiReceiveMessage(ConnectionId conn, message_t *msg);

#endif /* RUDICORE_H_ */
