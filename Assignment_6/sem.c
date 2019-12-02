
#include "sem.h"

static void handler () {
        //this is a dummy handler for initializations
}

//should be called only once in the program (per semaphore)
void sem_init (struct sem * s, int count) {
        // s->spinlock = 0;
        s->semaphore = count; //initialze the semaphore *s with the initial count

        int * mapped_area = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0 );
        if(mapped_area==MAP_FAILED) {
                fprintf(stderr,"Failed to mmap ANONYMOUS page[cv_init]: %s\n",strerror(errno));
                exit(EXIT_FAILURE);
        }

        spinlock * lock;
        lock=(spinlock *)(mapped_area+sizeof(spinlock)); /*important:make sure lock is fixed*/
        s->lock=*lock;

        s->prockBlockIndex = -1; // (no blocking processors)
        sigfillset (&s->mask_block);
        sigdelset (&s->mask_block, SIGUSR1); // removes SIGUSR1 from blocked signal list


        if(signal (SIGUSR1, handler)<0) {
                fprintf(stderr,"ERROR: Failed to signal handle: %s\n",strerror(errno));
                exit(EXIT_FAILURE);
        }
}


int sem_try (struct sem * s) {
        spin_lock(&s->lock);
        if (s->semaphore > 0) {
                s->semaphore--;
                spin_unlock(&s->lock);
                return 1;
        } else {
                spin_unlock(&s->lock);
                return 0;
        }
}

void sem_wait (struct sem * s) {
        //perform the P operation, blocking until successful
        while (1) {
                spin_lock(&s->lock);
                if (s->semaphore > 0) {
                        s->semaphore--;
                        spin_unlock(&s->lock);
                        // printf("FIRSTLOOP");

                        break;
                } else {
                        // printf("SECONDLOOP");
                        if(sigprocmask(SIG_BLOCK, &s->mask_block, NULL)<0) {
                                fprintf(stderr,"ERROR: Failed to examine and change blocked signals: %s\n",strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                        s->proc_block[s->prockBlockIndex] = my_procnum;
                        s->prockBlockIndex++;

                        spin_unlock(&s->lock);
                        if(sigsuspend (&s->mask_block)<0) {
                                fprintf(stderr,"ERROR: Failed to wait for signal: %s\n",strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                        if(sigprocmask(SIG_UNBLOCK, &s->mask_block, NULL)<0) {
                                fprintf(stderr,"ERROR: Failed to examine and change blocked signals: %s\n",strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                }
        }
}

void sem_inc (struct sem * s) {
        //perform the V operation, increment the semaphore by 1, if the semaphore
        //value is now positive, any sleeping tasks are awakened.
        spin_lock(&s->lock);
        s->semaphore++;
        if (s->semaphore == 1) {

                while (s->prockBlockIndex != -1) {
                        if(kill (pid_table[s->proc_block[s->prockBlockIndex]], SIGUSR1)<0) {
                                fprintf(stderr,"ERROR: Failed to send signal to process %d: %s\n",pid_table[s->proc_block[s->prockBlockIndex]], strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                        s->prockBlockIndex--;
                }

        }
        spin_unlock(&s->lock);
}
