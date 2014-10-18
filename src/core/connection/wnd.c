#include "wnd.h"

Window *createWindow(const uint32_t base, const uint32_t end) {
	Window *wnd = NULL;

	if (!(wnd = malloc(sizeof(Window))))
		ERREXIT("Cannot allocate memory for window");

	wnd->edge_mtx = createMutex();

	wnd->next_mtx = createMutex();

	wnd->base = base;

	wnd->end = end;

	wnd->next = base;

	return wnd;
}

void freeWindow(Window *wnd) {

	destroyMutex(wnd->next_mtx);

	destroyMutex(wnd->edge_mtx);

	free(wnd);
}

long double getWindowSpace(Window *wnd) {
	long double space;

	space = (wnd->end - wnd->next);

	return space;
}

void slideWindow(Window *wnd, const uint32_t offset) {

	lockMutex(wnd->edge_mtx);

	wnd->base = RUDP_NXTSEQN(wnd->base, offset);

	wnd->end = RUDP_NXTSEQN(wnd->end, offset);

	unlockMutex(wnd->edge_mtx);
}

void slideWindowNext(Window *wnd, const uint32_t offset) {
	lockMutex(wnd->next_mtx);

	wnd->next = RUDP_NXTSEQN(wnd->next, offset);

	unlockMutex(wnd->next_mtx);
}

short matchWindow(Window *wnd, const uint32_t value) {
	short match;

	lockMutex(wnd->edge_mtx);

	match = matchSequenceAgainstWindow(wnd->base, wnd->end, value);

	unlockMutex(wnd->edge_mtx);

	return match;
}
