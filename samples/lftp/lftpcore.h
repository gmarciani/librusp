#ifndef LFTPCORE_H_
#define LFTPCORE_H_

#include <stdlib.h>
#include <stdio.h>
#include "rusp.h"
#include "util/fileutil.h"
#include "util/stringutil.h"
#include "util/macroutil.h"

// Message Serialization
#define LFTP_HDRF 2
#define LFTP_HEAD 4
#define LFTP_BODY (4096 * 2)
#define LFTP_MSGS LFTP_HEAD + LFTP_BODY + 1
#define LFTP_PDELIM ";"

// Message Representation
#define LFTP_STR_HEAD LFTP_HEAD + LFTP_HDRF
#define LFTP_STR_BODY LFTP_BODY + 1
#define LFTP_STR_MSG LFTP_STR_HEAD + LFTP_STR_BODY

/* TYPES */
#define LFTP_TYP 3
#define LFTP_REQUEST 0
#define LFTP_SUCCESS 1
#define LFTP_BADRQST	2

/* ACTIONS */
#define LFTP_ACT 12
#define LFTP_GTCWD 0
#define LFTP_CHDIR 1
#define LFTP_LSDIR 2
#define LFTP_MKDIR 3
#define LFTP_RMDIR 4
#define LFTP_CPDIR 5
#define LFTP_MVDIR 6
#define LFTP_RETRF 7
#define LFTP_STORF 8
#define LFTP_RMFIL 9
#define LFTP_CPFIL 10
#define LFTP_MVFIL 11

/* MENU */
#define LFTP_MENU_CHOICES 13
#define LFTP_MENU_GTCWD 1
#define	LFTP_MENU_CHDIR 2
#define	LFTP_MENU_LSDIR 3
#define	LFTP_MENU_MKDIR 4
#define	LFTP_MENU_RMDIR 5
#define	LFTP_MENU_CPDIR 6
#define	LFTP_MENU_MVDIR 7
#define	LFTP_MENU_DWFILE 8
#define	LFTP_MENU_UPFILE 9
#define	LFTP_MENU_RMFILE 10
#define	LFTP_MENU_CPFILE 11
#define	LFTP_MENU_MVFILE 12
#define	LFTP_MENU_EXIT 13
#define LFTP_MENU_ERROR -1

/* GLOBAL VARIABLES */

extern int LFTP_DEBUG;

/* MESSAGE STRUCTURES */

typedef struct MsgHeader {
	int type;
	int action;
} MsgHeader;

typedef struct Message {
	MsgHeader header;
	char body[LFTP_BODY];
} Message;

/* SESSION STRUCTURES */

typedef struct Session {
	ConnectionId ctrlconn;
	ConnectionId dataconn;
	char cwd[PATH_MAX];
} Session;

typedef struct DataTransfer {
	ConnectionId conn;
	int fd;
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
