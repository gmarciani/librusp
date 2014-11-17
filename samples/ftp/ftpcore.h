#ifndef FTPCORE_H_
#define FTPCORE_H_

#include <stdlib.h>
#include <stdio.h>
#include "rusp.h"
#include "util/fileutil.h"
#include "util/stringutil.h"
#include "util/macroutil.h"

// Message Serialization
#define MSG_HDRF 2
#define MSG_HEAD 4
#define MSG_BODY (4096 * 2)
#define MSGSIZE MSG_HEAD + MSG_BODY + 1
#define MSG_PDELIM ";"

// Message Representation
#define MSG_STR_HEAD MSG_HEAD + MSG_HDRF
#define MSG_STR_BODY MSG_BODY + 1
#define MSG_STR MSG_STR_HEAD + MSG_STR_BODY

/* TYPES */

#define MSG_TYP 3
#define MSG_REQUEST 0
#define MSG_SUCCESS 1
#define MSG_BADRQST	2

/* ACTIONS */
#define MSG_ACT 12
#define MSG_GTCWD 0
#define MSG_CHDIR 1
#define MSG_LSDIR 2
#define MSG_MKDIR 3
#define MSG_RMDIR 4
#define MSG_CPDIR 5
#define MSG_MVDIR 6
#define MSG_RETRF 7
#define MSG_STORF 8
#define MSG_RMFIL 9
#define MSG_CPFIL 10
#define MSG_MVFIL 11

/* MENU */
#define MENU_CHOICES 13
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

/* GLOBAL VARIABLES */

extern int FTP_DEBUG;

/* MESSAGE STRUCTURES */

typedef struct MsgHeader {
	int type;
	int action;
} MsgHeader;

typedef struct Message {
	MsgHeader header;
	char body[MSG_BODY];
} Message;

/* SESSION STRUCTURES */

typedef struct Session {
	ConnectionId ctrlconn;
	ConnectionId dataconn;
	char cwd[PATH_MAX];
} Session;

typedef struct DataTransfer {
	ConnectionId conn;
	char path[PATH_MAX];
} DataTransfer;

/* MESSAGE */

Message createMessage(const int type, const int action, const char *body);

size_t serializeMessage(const Message msg, char *smsg);

void deserializeMessage(const char *smsg, Message *msg);

void messageToString(const Message msg, char *strmsg);

/* MESSAGE I/O */

ssize_t sendMessage(const ConnectionId conn, const Message msg);

ssize_t receiveMessage(const ConnectionId conn, Message *msg);

void printOutMessage(const ConnectionId conn, const Message msg);

void printInMessage(const ConnectionId conn, const Message msg);

/* MENU */

int runMenu(Session *session, Message *msg);


#endif /* FTPCORE_H_ */
