#ifndef __FIFO_H__
#define __FIFO_H__

#include "sem.h"

#define MYFIFO_BUFSIZ 4096

struct fifo {
	unsigned long buffer[MYFIFO_BUFSIZ];
	int next_read;
	int next_write;
	struct sem empty, full, mutex;
	spinlock FIFO_lock;
};

void fifo_init (struct fifo * f);

void fifo_wr (struct fifo * f, unsigned long d);

unsigned long fifo_rd (struct fifo * f);

#endif
