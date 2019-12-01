#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* actualsmear(char *addrMmap, const char *target, const char *replacement) {
        char *tmp;
        int distBtwnTargetToTarget, targetCount;
        int targetLength = strlen(target);
        char* addrStart = addrMmap;

        if ((!(targetLength == strlen(replacement))) || targetLength == 0 || target == NULL ) {
                printf("ERROR: TARGET and REPLACEMENT strings must exist and be the same length\n");
                exit(EXIT_FAILURE);
        }

        char* ins = addrMmap;
        for (targetCount = 0; tmp = strstr(ins, target); ++targetCount) {
                ins = tmp + targetLength;
        }

        while (targetCount--) {
                ins = strstr(addrMmap, target);
                distBtwnTargetToTarget = ins - addrMmap;
                for (int j = 0; j< targetLength; j++) {
                        addrMmap[distBtwnTargetToTarget+j] = replacement[j];
                }
                addrMmap += distBtwnTargetToTarget + targetLength;
        }
        return addrStart;
}

int main(int argc, char *argv[])
{
        char *addr;
        int fd;
        struct stat sb;

        if (argc < 4) {
                printf("ERROR: Improper arguments specified, appropriate structure is: smear TARGET REPLACEMENT file1 {file 2....}\n");
                exit(EXIT_FAILURE);
        }

        char* TARGET = argv[1];
        char* REPLACEMENT = argv[2];

        for(int i = 0; i < (argc-3); i++) {

                if((fd = open(argv[i+3], O_RDWR)) <0) {
                        fprintf(stderr, "ERROR: Could not open file '%s' to replace TARGET with REPLACEMENT due to '%s' \n", argv[i+3], strerror(errno));
                        exit(EXIT_FAILURE);
                }

                if (fstat(fd, &sb) < 0) {
                        fprintf(stderr, "ERROR: Could not stat file '%s' due to:%s \n", argv[i+3], strerror(errno));
                        exit(EXIT_FAILURE);
                }

                size_t length = sb.st_size;

                if ((addr = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
                        fprintf(stderr, "ERROR: Could not map file '%s': %s\n", argv[i+3], strerror(errno));
                        exit(EXIT_FAILURE);
                }

                addr = actualsmear(addr, TARGET, REPLACEMENT );

                if(munmap(addr, length) < 0) {
                        fprintf(stderr, "ERROR: Could not munmap file %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                }

                if(close(fd) < 0) {
                        fprintf(stderr, "ERROR: Could not close file '%s' due to %s\n",  argv[i+3], strerror(errno));
                        exit(EXIT_FAILURE);
                }

        }
        exit(EXIT_SUCCESS);
}
