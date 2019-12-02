#include "fifo.h"


void fifo_init (struct fifo * f) {
								f->next_read = 0;
								f->next_write = 0;
								sem_init (&f->empty, MYFIFO_BUFSIZ);
								sem_init (&f->full, 0);
								sem_init (&f->mutex, 1);
								f->FIFO_lock.p_lock=0;
}

void fifo_wr (struct fifo * f, unsigned long d) {
								spin_lock(&f->FIFO_lock);
								while (1) {
																sem_wait (&f->empty);
																if (sem_try (&f->mutex)) {
																								f->buffer[f->next_write] = d;
																								f->next_write++;
																								f->next_write %= MYFIFO_BUFSIZ;
																								sem_inc (&f->mutex);
																								sem_inc (&f->full);
																								break;
																} else {
																								sem_inc (&f->empty);
																}
								}
								 spin_unlock(&f->FIFO_lock);
}

unsigned long fifo_rd (struct fifo * f) {
		spin_lock(&f->FIFO_lock);
								unsigned long d;
								while (1) {
																sem_wait (&f->full);
																if (sem_try (&f->mutex)) {
																								d = f->buffer[f->next_read];
																								f->next_read++;
																								f->next_read %= MYFIFO_BUFSIZ;
																								sem_inc (&f->mutex);
																								sem_inc (&f->empty);
																								break;
																} else {
																								sem_inc (&f->full);
																}
								}
									spin_unlock(&f->FIFO_lock);
								return d;
}
