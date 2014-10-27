#include "wnd.h"

void initializeWindow(Window *wnd, const uint32_t base, const uint32_t end) {

	pthread_rwlock_init(&(wnd->rwlock), NULL);

	pthread_mutex_init(&(wnd->mtx), NULL);

	pthread_cond_init(&(wnd->cnd), NULL);

	wnd->base = base;

	wnd->end = end;

	wnd->next = base;
}

void destroyWindow(Window *wnd) {

	if (pthread_rwlock_destroy(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot destroy window read-write lock.");

	if (pthread_mutex_destroy(&(wnd->mtx)) > 0)
		ERREXIT("Cannot destroy window mutex.");

	if (pthread_cond_destroy(&(wnd->cnd)) > 0)
		ERREXIT("Cannot destroy window condition variable.");

	wnd->base = 0;

	wnd->end = 0;

	wnd->next = 0;
}

uint32_t getWindowBase(Window *wnd) {
	uint32_t wndbase;

	if (pthread_rwlock_rdlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	wndbase = wnd->base;

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return wndbase;
}

uint32_t getWindowEnd(Window *wnd) {
	uint32_t wndend;

	if (pthread_rwlock_rdlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	wndend = wnd->end;

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return wndend;
}

uint32_t getWindowNext(Window *wnd) {
	uint32_t wndnext;

	if (pthread_rwlock_rdlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	wndnext = wnd->next;

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return wndnext;
}

long getWindowSpace(Window *wnd) {
	long space;

	if (pthread_rwlock_rdlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	space = (wnd->end - wnd->next);

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return space;
}

void slideWindow(Window *wnd, const uint32_t offset) {
	if (pthread_rwlock_wrlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	wnd->base = RUDP_NXTSEQN(wnd->base, offset);

	wnd->end = RUDP_NXTSEQN(wnd->end, offset);

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	if (pthread_cond_broadcast(&(wnd->cnd)) > 0)
		ERREXIT("Cannot broadcast condition variable.");
}

void slideWindowNext(Window *wnd, const uint32_t offset) {
	if (pthread_rwlock_wrlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire write-lock.");

	wnd->next = RUDP_NXTSEQN(wnd->next, offset);

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");
}

short matchWindow(Window *wnd, const uint32_t value) {
	short match;

	if (pthread_rwlock_rdlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot acquire read-lock.");

	match = matchSequenceAgainstWindow(wnd->base, wnd->end, value);

	if (pthread_rwlock_unlock(&(wnd->rwlock)) > 0)
		ERREXIT("Cannot release read-write lock.");

	return match;
}

void waitWindowSpace(Window *wnd, const long space) {
	if (pthread_mutex_lock(&(wnd->mtx)) > 0)
		ERREXIT("Cannot lock mutex.");

	while (getWindowSpace(wnd) < space)
		if (pthread_cond_wait(&(wnd->cnd), &(wnd->mtx)) > 0)
			ERREXIT("Cannot wait for condition variable.");

	if (pthread_mutex_unlock(&(wnd->mtx)) > 0)
		ERREXIT("Cannot unlock mutex.");
}
