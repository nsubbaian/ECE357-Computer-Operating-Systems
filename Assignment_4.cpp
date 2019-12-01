#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <unistd.h>

long buffersize = 4096;
char *pattern = NULL;
int processedfiles = 0;
int processedbytes = 0;
int inputfile;

void siginthandler(int sig){
        fprintf(stderr,"\n\nProcess interrupted by SIGINT\n");
        fprintf(stderr,"Files processed: %d Bytes processed: %d \n",processedfiles,processedbytes);
        exit(EXIT_FAILURE);
}

void sigpipehandler(int sig){
        fprintf(stderr,"Broken Pipe\n");
        exit(EXIT_FAILURE);
}

int main(int argc, char **argv){

        int m = -1;
        int n = -1;

        if(argc < 3) {
                printf("ERROR: Improper arguments specified, appropriate structure is: catgrepmore pattern inputfile1 [...inputfile2...]\n");
                exit(EXIT_FAILURE);
        }

        signal(SIGINT, siginthandler);
        signal(SIGPIPE, sigpipehandler);

        pattern = argv[1];

        for(int i = 2; i<argc; i++) {

                int fd1pipe[2]; //grep
                int fd2pipe[2]; //more

                if (pipe(fd1pipe) < 0 || pipe(fd2pipe) <0) {
                        fprintf(stderr,"ERROR: Could not create Pipes:%s",strerror(errno));
                        exit(EXIT_FAILURE);
                }

                if( (inputfile = open(argv[i], O_RDONLY))<0) {
                        fprintf(stderr,"ERROR: Could not open file %s: %s\n", argv[i], strerror(errno));
                        exit(EXIT_FAILURE);
                }

                pid_t grepPID, morePID;
                int grepstatus, morestatus;

                if((grepPID = fork()) < 0) {
                        fprintf(stderr,"ERROR: Could not fork for grep command: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                } else if(grepPID == 0) {

                        if(inputfile!=0) {
                                if((close(inputfile))<0) {
                                        fprintf(stderr,"ERROR: Could not close input file: %s\n", strerror(errno));
                                        exit(EXIT_FAILURE);
                                }
                        }

                        if(close(fd1pipe[1]) < 0 || close(fd2pipe[0])  < 0) {
                                fprintf(stderr,"ERROR: Could not close unused pipes in grep: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }


                        if((dup2(fd1pipe[0], 0) < 0) || (dup2(fd2pipe[1], 1) < 0)) {
                                fprintf(stderr,"ERROR: Could not dup2 in grep: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                        if (close(fd1pipe[0]) <0 ||  close(fd2pipe[1]) <0) {
                                fprintf(stderr,"ERROR: Could not close pipes in grep: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                        if(execlp("grep", "grep", pattern, NULL) < 0) {
                                fprintf(stderr,"ERROR: Could not exec grep: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                }

                if((morePID = fork()) == -1) {
                        fprintf(stderr,"ERROR: Could not fork for 'more' command: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                } else if(morePID == 0) {

                        if(inputfile!=0) {
                                if((close(inputfile))<0) {
                                        fprintf(stderr,"ERROR: Could not close input file: %s\n", strerror(errno));
                                        exit(EXIT_FAILURE);
                                }
                        }

                        if(close(fd1pipe[1]) < 0 || close(fd1pipe[0])  < 0|| close(fd2pipe[1])  < 0) {
                                fprintf(stderr,"ERROR: Could not close unused pipes in 'more': %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                        if((dup2(fd2pipe[0], 0) < 0)) {
                                fprintf(stderr,"ERROR: Could not dup2 in 'more': %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }


                        if (close(fd2pipe[0]) <0 ) {
                                fprintf(stderr,"ERROR: Could not close pipes in 'more': %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                        if(execlp("more", "more", NULL) < 0) {
                                fprintf(stderr,"ERROR: Could not exec 'more': %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                }

                if(grepPID != 0 && morePID != 0) {

                        if(close(fd1pipe[0]) < 0 || close(fd2pipe[1])  < 0|| close(fd2pipe[0])  < 0) {
                                fprintf(stderr,"ERROR: Could not close unused pipes in parent process: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }

                        char buffer[buffersize];
                        while ((n = read(inputfile, buffer, sizeof(char)*buffersize)) > 0) {
                                if (n <0) {
                                        fprintf(stderr, "ERROR: Could not read from input file %s:%s\n", argv[i], strerror(errno));
                                        exit(EXIT_FAILURE);
                                } else if(n == 0) {
                                        break;
                                } else {
                                        int written = 0;
                                        while(written < n) {
                                                if ((m = write(fd1pipe[1], buffer+written, n-written))<=0) {
                                                        if(errno==EPIPE) {
                                                                break;
                                                        }
                                                        fprintf(stderr, "ERROR: Could not write end of pipe 1:%s\n", strerror(errno));
                                                        exit(EXIT_FAILURE);
                                                }
                                                written += m;
                                        }
                                        processedbytes = written;
                                }
                        }


                        if(close(fd1pipe[1]) < 0 ) {
                                fprintf(stderr,"ERROR: Could not close unused pipes in parent process: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }


                        if(inputfile!=0) {
                                if((close(inputfile))<0) {
                                        fprintf(stderr,"ERROR: Could not close input file: %s\n", strerror(errno));
                                        exit(EXIT_FAILURE);
                                }
                        }

                        if (waitpid(grepPID, &grepstatus, 0)<0|| waitpid(morePID, &morestatus, 0) < 0) {
                                fprintf(stderr,"ERROR: Error in waiting for child process: %s\n", strerror(errno));
                                exit(EXIT_FAILURE);
                        }


                           if(grepstatus!=0) {
                                   if(WIFSIGNALED(grepstatus) && WTERMSIG(morestatus)!=SIGPIPE) {
                                           fprintf(stderr, "Signal number that caused grep to terminate: %d\n", WTERMSIG(morestatus));
                                   }
                           } else {
                                   if(WIFEXITED(grepstatus)) {
                                           fprintf(stderr, "Grep Exit Status is: %d\n", WEXITSTATUS(grepstatus));
                                   }
                           }

                           if(morestatus!=0) {
                                   if(WIFSIGNALED(morestatus) && WTERMSIG(morestatus)!=SIGPIPE) {
                                           fprintf(stderr, "Signal number that caused 'more' to terminate: %d\n", WTERMSIG(morestatus));
                                   }
                           } else {
                                   if(WIFEXITED(grepstatus)) {
                                           fprintf(stderr, "'more' Exit Status is: %d\n", WEXITSTATUS(morestatus));
                                   }
                           }

                }
                processedfiles++;
        }
        return 0;
}
