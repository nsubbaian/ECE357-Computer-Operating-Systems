#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

int main(int argc, char **argv) {

        int buffer_size = 1024;
        int gresult, fdin;
        int fdout = 1;
        int Num_inputs = 0;
        int no_input = 0;
        int outflag = 1;
        char* outfile = "stdout";
        char* infile;
        int m = -1;
        int n= -1;
        int Oseen = 0;
        int Bseen = 0;

        opterr = 0;

        while((gresult = getopt(argc, argv, "b:o:")) != -1) {
                switch(gresult) {
                case 'b':
                        buffer_size = atoi(optarg);

                        if (Bseen++>0) {
                                printf("WARNING: Multiple buffer sizes were entered\n");
                        }

                        if (buffer_size <1) {
                                fprintf(stderr, "ERROR: Invalid buffer size in bytes was entered after the '-b'\n");
                                exit(EXIT_FAILURE);
                        }
                        break;

                case 'o':

                        outfile = optarg;
                        outflag = 0;
                        fdout = 0;

                        if (Oseen++>0) {
                                printf("WARNING: Multiple output files were specified\n");
                        }

                        fdout=open(outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
                        if (fdout < 0) {
                                fprintf(stderr, "ERROR: Could not open output file %s for writing due to %s\n", outfile, strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                        break;

                case '?':
                        fprintf(stderr,"ERROR: Invalid option of '%s' was entered\n", argv[optind-1]);
                        exit(EXIT_FAILURE);
                default:
                        printf("%s\n","ERROR: Invalid arguments entered for getopt function\n" );
                        exit(EXIT_FAILURE);
                }
        }

        char* buffer = malloc(sizeof(char)* buffer_size);

        Num_inputs= argc-optind;

        if (Num_inputs <= 0) {
                Num_inputs=1;
                no_input = 1;
        }


        for (int i = 0; i < Num_inputs; i++) {
                if (no_input == 1) {
                        infile = "-";
                } else{
                        infile = argv[optind + i];
                }

                if ((strcmp(infile,"-")) == 0) {
                        fdin = 0;
                        infile = "stdin";
                }  else{
                        fdin = open(infile, O_RDONLY);
                        if(fdin <0) {
                                fprintf(stderr, "ERROR: File'%s' could not be open for reading due to '%s' \n", infile, strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                }

                while ((n = read(fdin, buffer, sizeof(char)*buffer_size)) > 0) {
                        if (n <0) {
                                fprintf(stderr, "ERROR: Could not read from input file '%s' due to %s\n", infile, strerror(errno));
                                exit(EXIT_FAILURE);
                        } else {
                                int written = 0;
                                while(written < n) {
                                        if ((m = write(fdout, buffer+written, n))<0) {
                                                fprintf(stderr, "ERROR: Could not write to output file %s due to %s\n", outfile, strerror(errno));
                                                exit(EXIT_FAILURE);
                                        }
                                        written += m;
                                }
                        }
                }

                if ((fdin != 0) && (close(fdin) < 0)) {
                        fprintf(stderr, "ERROR: Could not close input file %s due to %s\n", infile, strerror(errno));
                        exit(EXIT_FAILURE);

                }
        }

        if ((fdout != 1) && (close(fdout) < 0)) {
                fprintf(stderr, "ERROR: Could not close output file %s due to %s\n", outfile, strerror(errno));
                exit(EXIT_FAILURE);
        }

        return(0);

}
