#include "rudpmailbox.h"

MailBox *createMailBox(const int sock, const uint32_t sndisn, const uint32_t rcvisn, uint16_t wndsize, const struct itimerspec *timervalue) {

}

void freeMailBox(MailBox *mailbox) {

}

size_t writeToOutBuffer(const char *msg, const size_t writesize) {

}

size_t readFromInBuffer(char *msg, const size_t readsize) {

}
