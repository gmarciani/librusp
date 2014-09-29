#include "rudpinbox.h"

SegmentInbox *createInbox(const uint32_t wnds) {

}

void freeInbox(SegmentInbox *inbox) {

}

void setListeningInbox(SegmentInbox *inbox) {

}

void submitSegmentToInbox(SegmentInbox *inbox, const Segment sgm) {

}

Segment readInboxSegment(SegmentInbox *inbox) {
	Segment sgm;

	return sgm;
}

char *readInboxBuffer(SegmentInbox *inbox, const size_t size) {
	char *msg = NULL;


	return msg;
}

void _slideInboxWindow(SegmentInbox *inbox) {

}

void _removeInboxElement(SegmentInbox *inbox, InboxElement *elem) {

}

char *inboxToString(const SegmentInbox inbox) {
	return NULL;
}
