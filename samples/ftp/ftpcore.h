#ifndef FTPCORE_H_
#define FTPCORE_H_

#include <stdlib.h>
#include <stdio.h>
#include "rusp.h"
#include "util/fileutil.h"
#include "util/stringutil.h"

// Message Sizes
#define MSG_HEAD 4
#define MSG_BODY (4096 * 2) + 2
#define MSG_HDRF 2

// Message Serialization
#define MSGSIZE MSG_HEAD + MSG_BODY
#define MSG_FIELD_DELIM "\\"
#define OBJECT_FIELDS_DELIMITER ";"

// Type
#define MSG_REQUEST 1
#define MSG_SUCCESS 2
#define MSG_BADRQST	3

// Action
#define MSG_GTCWD 1
#define MSG_CHDIR 2
#define MSG_LSDIR 3
#define MSG_MKDIR 4
#define MSG_RMDIR 5
#define MSG_CPDIR 6
#define MSG_MVDIR 7
#define MSG_RETRF 8
#define MSG_STORF 9
#define MSG_RMFIL 10
#define MSG_CPFIL 11
#define MSG_MVFIL 12

// Menu
#define MENU_CHOICES 13

// Menus Choices
#define MENU_GTCWD 1
#define	MENU_CHDIR 2
#define	MENU_LSDIR 3
#define	MENU_MKDIR 4
#define	MENU_RMDIR 5
#define	MENU_CPDIR 6
#define	MENU_MVDIR 7
#define	MENU_DWFILE 8
#define	MENU_UPFILE 9
#define	MENU_RMFILE 10
#define	MENU_CPFILE 11
#define	MENU_MVFILE 12
#define	MENU_EXIT 13
#define MENU_ERROR -1

#define TR_UPL 1
#define TR_DWL 2

// Message Header
typedef struct MsgHeader {
	int type;
	int action;
} MsgHeader;

// Message Structure
typedef struct Message {
	MsgHeader header;
	char body[MSG_BODY];
} Message;

// Download/Upload
typedef struct DataTransfer {
	ConnectionId conn;
	char path[PATH_MAX];
} DataTransfer;

// Session
typedef struct Session {
	ConnectionId ctrlconn;
	ConnectionId dataconn;
	char cwd[PATH_MAX];
} Session;

/* MESSAGE */

Message createMessage(const int type, const int action, const char *body);

size_t serializeMessage(const Message msg, char *smsg);

void deserializeMessage(const char *smsg, Message *msg);

/* MESSAGE I/O */

ssize_t sendMessage(const ConnectionId conn, const Message msg);

ssize_t receiveMessage(const ConnectionId conn, Message *msg);

/* MENU */

int runMenu(Session *session, Message *msg);


#endif /* FTPCORE_H_ */
