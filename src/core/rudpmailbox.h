#ifndef _RUDPMAILBOX_H_
#define _RUDPMAILBOX_H_

#include <time.h>
#include <pthread.h>
#include "rudpsegmentlist.h"
#include "rudpsegment.h"

#define MAILBOX_RCV 	0
#define MAILBOX_SND		1

#define MAILBOX_BUFFER	65535

typedef struct MailBox {
	unsigned int state;
	uint32_t snduna;
	uint32_t sndnxt;
	uint32_t sndwnd;	
	uint32_t rcvnxt;
	uint32_t rcvwnd;
	char sndbuff[MAILBOX_BUFFER];	
	char rcvbuff[MAILBOX_BUFF];
	uint16_t sndbuffs;
	uint16_t rcvbuffs;	
	int sock;
	timer_t timer;
	struct itimerspec timeout;
	pthread_cond_t  mailbox_cnd;
	pthread_mutex_t mailbox_mtx;
} MailBox;

MailBox *createMailBox(const int sock, const uint32_t sndisn, const uint32_t rcvisn, uint16_t wndsize, const struct itimerspec timeout);

void freeMailBox(MailBox *mailbox);

size_t writeToOutBuffer(const char *msg, const size_t writesize);

size_t readFromInBuffer(char *msg, const size_t readsize);



#endif /* _RUDPMAILBOX_H_ */
