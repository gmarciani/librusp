#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> 
#include "../protocol/util/stringutil.h"

#define MAX_INPIPE 2;

typedef struct mailbox_t {
	char *outbox;
	size_t outboxs;
} mailbox_t;

void send(mailbox_t *mail, const char *str);

static void *processOutbox(void *arg);

int main(void) {
	mailbox_t mail;

	send(&mail, "0123456789");
	
	return(EXIT_SUCCESS);	
}

void send(mailbox_t *mail, const char *str) {
	printf("Sending: %s\n", str);
	pthread_t snd;
	void *sndres;

	mail->outbox = stringDuplication(str);
	mail->outboxs = strlen(mail->outbox);

	printf("Oubox: %s\n", mail->outbox);

	if (pthread_create(&snd, NULL, processOutbox, mail) != 0) {
		fprintf(stderr, "Cannot create sender thread.\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_join(snd, &sndres) != 0) {
		fprintf(stderr, "Cannot join sender thread.\n");
		exit(EXIT_FAILURE);
	}

	printf("%ld bytes correctly sent!\n", (long) sndres);	
	printf("Oubox: %s\n", mail->outbox);
}

static void *processOutbox(void *arg) {
	mailbox_t *mail = (mailbox_t *) arg;
	long int sent = 0;
	int i;

	for (i = mail->outboxs - 1; i >= 0; i--) {
		mail->outbox[i] = '\0';
		mail->outboxs--;
		sent++;
	}

	return (void *) sent;
}
