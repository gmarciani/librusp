#include "wnd.h"

Window *createWindow(const uint32_t base, const uint32_t end) {
	Window *wnd = NULL;

	if (!(wnd = malloc(sizeof(Window))))
		ERREXIT("Cannot allocate memory for window");

	wnd->mtx = createMutex();

	wnd->cnd = createConditionVariable();

	wnd->rwlock = createRWLock();

	wnd->base = base;

	wnd->end = end;

	wnd->next = base;

	return wnd;
}

void freeWindow(Window *wnd) {

	freeRWLock(wnd->rwlock);

	destroyMutex(wnd->mtx);

	destroyConditionVariable(wnd->cnd);

	free(wnd);
}

uint32_t getWindowBase(Window *wnd) {
	uint32_t wndbase;

	lockRead(wnd->rwlock);

	wndbase = wnd->base;

	unlockRWLock(wnd->rwlock);

	return wndbase;
}

uint32_t getWindowEnd(Window *wnd) {
	uint32_t wndend;

	lockRead(wnd->rwlock);

	wndend = wnd->end;

	unlockRWLock(wnd->rwlock);

	return wndend;
}

uint32_t getWindowNext(Window *wnd) {
	uint32_t wndnext;

	lockRead(wnd->rwlock);

	wndnext = wnd->next;

	unlockRWLock(wnd->rwlock);

	return wndnext;
}

long getWindowSpace(Window *wnd) {
	long space;

	lockRead(wnd->rwlock);

	space = (wnd->end - wnd->next);

	unlockRWLock(wnd->rwlock);

	return space;
}

void slideWindow(Window *wnd, const uint32_t offset) {
	lockWrite(wnd->rwlock);

	wnd->base = RUDP_NXTSEQN(wnd->base, offset);

	wnd->end = RUDP_NXTSEQN(wnd->end, offset);

	unlockRWLock(wnd->rwlock);

	broadcastConditionVariable(wnd->cnd);
}

void slideWindowNext(Window *wnd, const uint32_t offset) {
	lockWrite(wnd->rwlock);

	wnd->next = RUDP_NXTSEQN(wnd->next, offset);

	unlockRWLock(wnd->rwlock);
}

short matchWindow(Window *wnd, const uint32_t value) {
	short match;

	lockRead(wnd->rwlock);

	match = matchSequenceAgainstWindow(wnd->base, wnd->end, value);

	unlockRWLock(wnd->rwlock);

	return match;
}

void waitWindowSpace(Window *wnd, const long space) {
	lockMutex(wnd->mtx);

	while (getWindowSpace(wnd) < space)
		waitConditionVariable(wnd->cnd, wnd->mtx);

	unlockMutex(wnd->mtx);
}
