#ifndef __SEM_H__
#define __SEM_H__

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
 #include <unistd.h>

#include "spinlock.h"

#define N_PROC 64

extern int my_procnum;
extern pid_t * pid_table;

struct sem {
								spinlock lock;
								int semaphore;
								int prockBlockIndex;
								int proc_block[N_PROC];
								sigset_t mask_block;
};

void sem_init (struct sem * s, int count);

int sem_try (struct sem * s);

void sem_wait (struct sem * s);

void sem_inc (struct sem * s);

#endif
