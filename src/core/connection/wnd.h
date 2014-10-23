#ifndef WND_H_
#define WND_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "../segment/seqn.h"
#include "../../util/macroutil.h"

typedef struct Window {
	uint32_t base;
	uint32_t end;
	uint32_t next;

	pthread_rwlock_t rwlock;
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
} Window;

void initializeWindow(Window *wnd, const uint32_t base, const uint32_t end);

void destroyWindow(Window *wnd);

uint32_t getWindowBase(Window *wnd);

uint32_t getWindowEnd(Window *wnd);

uint32_t getWindowNext(Window *wnd);

long getWindowSpace(Window *wnd);

void slideWindow(Window *wnd, const uint32_t offset);

void slideWindowNext(Window *wnd, const uint32_t offset);

short matchWindow(Window *wnd, const uint32_t value);

void waitWindowSpace(Window *wnd, const long space);

#endif /* WND_H_ */
