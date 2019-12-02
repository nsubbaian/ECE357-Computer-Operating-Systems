#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "fifo.h"

int my_procnum;
pid_t * pid_table;

int main (int argc, char ** argv) {
        struct fifo * f;
        int i, j, numberOfWriters = 50, numberOfIterations = 50;

        unsigned long datum;

        if ((f = (struct fifo *) mmap (NULL, sizeof (struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))== MAP_FAILED) {
                fprintf (stderr, "ERROR: mmap() failure: %s\n", strerror (errno));
                return -1;
        }

        if ((pid_table = (pid_t *) mmap (NULL, ((sizeof (pid_t)) * N_PROC), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))== MAP_FAILED) {
                fprintf (stderr, "ERROR: mmap() failure: %s\n", strerror (errno));
                return -1;
        }

        fifo_init (f);

        for (i = 0; i < numberOfWriters; ++i) {
                pid_table[i] = fork ();

                if (pid_table[i] < 0) {
                        fprintf (stderr, "ERROR: fork() failure: %s\n", strerror (errno));
                        return -1;
                } else if (pid_table[i] == 0) {
                        my_procnum = i;
                        unsigned long writeBuf[numberOfIterations];
                        for (j = 0; j < numberOfIterations; ++j) {
                                writeBuf[j] = j + getpid()*10000;
                                pid_table[i] = getpid ();
                                fifo_wr(f, writeBuf[j]);
                                // fprintf (stderr, "Process %d wrote %lu to FIFO\n", pid_table[i], writeBuf[j]);
                        }
                        // fprintf(stderr, "Writer %d completed\n",i);
                        return 0;
                }
        }
        fprintf(stderr,"ALL %d Writers done\n", numberOfWriters);

        pid_table[numberOfWriters] = fork ();

        if (pid_table[numberOfWriters] < 0) {
                fprintf (stderr, "ERROR: fork() failure: %s\n", strerror (errno));
                return -1;
        } else if (pid_table[numberOfWriters] == 0) {
                pid_table[numberOfWriters] = getpid ();
                my_procnum = numberOfWriters;

                for (i = 0; i < numberOfWriters * numberOfIterations; ++i) {
                        datum = fifo_rd (f);
                        // fprintf (stderr, "read %lu from FIFO on run %d\n", datum, i);
                }
                fprintf(stderr,"ALL readers done\n");
                return 0;
        }

        for (i = 0; i < (numberOfWriters + 1); ++i) {
                if (waitpid (pid_table[i], NULL, 0) < 0) {
                        fprintf (stderr, "ERROR: child process return failure: %s\n", strerror (errno));
                        return -1;
                }
        }

        return 0;
}
