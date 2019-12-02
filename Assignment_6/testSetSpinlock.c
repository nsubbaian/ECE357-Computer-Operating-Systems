#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "spinlock.h"

int main(int argc, char * argv[]) {

        if(argc!=3) {
                fprintf(stderr,"ERROR:  Specify the number of processes and the number of iterations after %s\n",argv[0]);
                exit(EXIT_FAILURE);
        }

        long long unsigned int ProcessNumber = atoll(argv[1]), IterationNumber = atoll(argv[2]);

        fprintf (stderr, "Number of Processes = %llu\n", ProcessNumber);
        fprintf (stderr, "Number of Iterations = %llu\n", IterationNumber);

        int * mappedRegion = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0 );
        int * mappedRegion2 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0 );

        if(mappedRegion==MAP_FAILED || mappedRegion2==MAP_FAILED) {
                fprintf(stderr,"ERROR: Failed to mmap ANONYMOUS page(s): %s\n",strerror(errno));
                exit(EXIT_FAILURE);
        }

        mappedRegion[0] = 0;
        mappedRegion2[0] = 0;

        spinlock * lock;
        lock=(spinlock *)(mappedRegion+sizeof(spinlock));
        lock->p_lock= mappedRegion[1];

        pid_t pids[ProcessNumber];

        for (int i = 0; i < ProcessNumber; i++) {
                if ((pids[i] = fork()) < 0) {
                        fprintf (stderr, "ERROR: Failed to fork for Process Number %d: %s\n", i, strerror (errno));
                        return EXIT_FAILURE;
                }
                if (pids[i] == 0) {
                        for (int jk = 0; jk < IterationNumber; jk++) {
                                mappedRegion2[0]++;
                        }

                        spin_lock(lock);
                        for (int j = 0; j < IterationNumber; j++) {
                                mappedRegion[0]++;
                        }
                        spin_unlock(lock);
                        exit(0);
                }
        }

        for (int ijk = 0; ijk < ProcessNumber; ijk++) {
                if (waitpid (pids[ijk], NULL, 0) < 0) {
                        fprintf (stderr, "ERROR: waitpid failed for reason of: %s\n", strerror (errno));
                }
        }

        printf ("(No. of Processes)*(No. of Iterations):\t%llu\n", ProcessNumber * IterationNumber);
        fprintf(stderr,"With mutex protection:\t\t\t%d\n", mappedRegion[0]);
        fprintf(stderr,"Without mutex protection:\t\t%d\n", mappedRegion2[0]);

}
