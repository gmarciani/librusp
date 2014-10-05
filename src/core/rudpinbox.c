#include "rudpinbox.h"

Inbox *createInbox(const uint32_t wndb, const uint32_t wnds) {
	Inbox *inbox = NULL;

	if (wnds == 0)
		ERREXIT("Cannot create inbox with window size set to zero.");

	if (!(inbox = malloc(sizeof(Inbox))))
		ERREXIT("Cannot allocate memory for inbox.");

	memset(inbox, 0, sizeof(Inbox));

	if (!(inbox->inbox_mtx = malloc(sizeof(pthread_mutex_t))) ||
		!(inbox->inbox_cnd = malloc(sizeof(pthread_cond_t))))
		ERREXIT("Cannot allocate memory for inbox resources.");	

	initializeMutex(inbox->inbox_mtx);

	initializeConditionVariable(inbox->inbox_cnd);

	inbox->sgmbuff = createSegmentList();

	inbox->userbuff = createBuffer();

	inbox->wndb = wndb;

	inbox->wnde = RUDP_NXTSEQN(wndb, wnds * RUDP_PLDS);

	inbox->wnds = wnds;

	inbox->push = 0;

	return inbox;
}

void freeInbox(Inbox *inbox) {

	destroyMutex(inbox->inbox_mtx);

	destroyConditionVariable(inbox->inbox_cnd);

	freeSegmentList(inbox->sgmbuff);

	freeBuffer(inbox->userbuff);

	free(inbox);
}

void submitSegmentToInbox(Inbox *inbox, const Segment sgm) {

	if (sgm.hdr.seqn >= inbox->wndb && sgm.hdr.seqn <= inbox->wnde) {

		if (sgm.hdr.seqn == inbox->wndb) {
			
			inbox->wndb = RUDP_NXTSEQN(inbox->wndb, sgm.hdr.plds);

			inbox->wnde = RUDP_NXTSEQN(inbox->wnde, sgm.hdr.plds);

			if (sgm.hdr.plds > 0)
				writeToBuffer(inbox->userbuff, sgm.pld, sgm.hdr.plds);

			if (sgm.hdr.ctrl & RUDP_PSH) {
				inbox->push = 1;
				inbox->pushsize = inbox->userbuff->csize;
			}

			int flag = 1;
			while (flag) {
				
				flag = 0;

				SegmentListElement *curr = inbox->sgmbuff->head;

				while (curr) {

					if (curr->segment->hdr.seqn == inbox->wndb) {
		
						flag = 1;
						
						inbox->wndb = RUDP_NXTSEQN(inbox->wndb, curr->segment->hdr.plds);

						inbox->wnde = RUDP_NXTSEQN(inbox->wnde, curr->segment->hdr.plds);

						if (curr->segment->hdr.plds > 0)
							writeToBuffer(inbox->userbuff, curr->segment->pld, curr->segment->hdr.plds);

						if (curr->segment->hdr.ctrl & RUDP_PSH) {
							inbox->push = 1;
							inbox->pushsize = inbox->userbuff->csize;
						}

						removeElementFromSegmentList(inbox->sgmbuff, curr);

						break;

					}
					
					curr = curr->next;
				}
			}			

		} else {

			addSegmentToSegmentList(inbox->sgmbuff, sgm);
		}
	}
}

char *readInboxBuffer(Inbox *inbox, const size_t size) {
	char *msg = NULL;
	size_t sizeToRead;

	sizeToRead = (size < inbox->pushsize) ? size : inbox->pushsize;

	msg = readFromBuffer(inbox->userbuff, sizeToRead);

	return msg;
}

char *inboxToString(Inbox *inbox) {
	char *strinbox = NULL;
	char *strsgmbuff = NULL;
	char *struserbuff = NULL;

	strsgmbuff = segmentListToString(inbox->sgmbuff);

	struserbuff = bufferToString(inbox->userbuff);	

	if (!(strinbox = malloc(sizeof(char) * (102 + strlen(strsgmbuff) + strlen(struserbuff) + 1))))
		ERREXIT("Cannot allocate memory for inbox string representation.");

	sprintf(strinbox, "wndb:%u wnde:%u wnds:%u psh:%u pshsize:%u\nsgmbuff:\n%s\nuserbuff:\n%s", inbox->wndb, inbox->wnde, inbox->wnds, inbox->push, inbox->pushsize, strsgmbuff, struserbuff);

	free(strsgmbuff);

	free(struserbuff);	

	return strinbox;
}
