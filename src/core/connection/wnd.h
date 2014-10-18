#ifndef WND_H_
#define WND_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "../segment/seqn.h"
#include "../../util/threadutil.h"

typedef struct Window {
	uint32_t base;
	uint32_t end;
	uint32_t next;

	pthread_mutex_t *edge_mtx;
	pthread_mutex_t *next_mtx;
} Window;

Window *createWindow(const uint32_t base, const uint32_t end);

void freeWindow(Window *wnd);

long double getWindowSpace(Window *wnd);

void slideWindow(Window *wnd, const uint32_t offset);

void slideWindowNext(Window *wnd, const uint32_t offset);

short matchWindow(Window *wnd, const uint32_t value);

#endif /* WND_H_ */
