#include "rudpinbox.h"

SegmentInbox createInbox(const unsigned long isn, const unsigned long wndsize) {
	SegmentInbox inbox;

	return inbox;
}

void freeInbox(SegmentInbox *inbox) {

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
