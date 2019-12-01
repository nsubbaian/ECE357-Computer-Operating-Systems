#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

int directorySearch(char* dir, int flag){
        struct dirent *readopendirectory;
        DIR *opendirectory;

        if (!(opendirectory = opendir (dir))) {
                fprintf(stderr, "ERROR: Could not open directory %s: %s\n", dir, strerror(errno));
                return -1;
        }
        errno = 0;
        while ((readopendirectory = readdir(opendirectory)) != NULL) {
                const char *directoryname;
                char path[PATH_MAX];

                if (!flag) {
                         if (((snprintf(path, sizeof(path), "%s/%s", dir, readopendirectory->d_name))<0)) {
                           fprintf(stderr, "ERROR: PATH_MAX was exceeded for %s: %s\n", dir, strerror(errno));
                           return -1;
                         }
                        directoryname = readopendirectory->d_name;
                }else{
                        sprintf(path, "%s", dir);
                        directoryname = dir;
                }

                if (strcmp(directoryname, ".") != 0 && strcmp(directoryname, "..") != 0) {
                        struct stat st1;
                        if (lstat(path, &st1) < 0) {
                                fprintf(stderr, "ERROR: Could not stat file %s: %s\n", path, strerror(errno));
                                return -1;
                        }

                        ino_t ino = st1.st_ino;
                        printf("%*lu", 9, ino);

                        blkcnt_t blocks = st1.st_blocks;
                        printf("%*lu ", 7, blocks/2);

                        mode_t mode = st1.st_mode;
                        int isdir = S_ISDIR(mode);
                        int issymlink = S_ISLNK(mode);
                        int isregfile = S_ISREG(mode);

                        if (isdir) {
                                printf("%s","d" );
                        }else if(issymlink) {
                                printf("%s","l" );
                        }else if(isregfile) {
                                printf("%s", "-");
                        }

                        printf( (mode & S_IRUSR) ? "r" : "-"); /* R for owner */
                        printf( (mode & S_IWUSR) ? "w" : "-"); /* W for owner */
                        printf( (mode & S_IXUSR) ? "x" : "-"); /* X for owner */
                        printf( (mode & S_IRGRP) ? "r" : "-"); /* R for group */
                        printf( (mode & S_IWGRP) ? "w" : "-"); /* W for group */
                        printf( (mode & S_IXGRP) ? "x" : "-"); /* X for group */
                        printf( (mode & S_IROTH) ? "r" : "-"); /* R for other */
                        printf( (mode & S_IWOTH) ? "w" : "-"); /* W for other */
                        printf( (mode & S_IXOTH) ? "x" : "-"); /* X for other */

                        nlink_t nlink = st1.st_nlink;
                        printf("%*lu", 4, nlink);


                        uid_t userid = st1.st_uid;
                        struct passwd *pwd;
                        pwd = getpwuid(userid);
                        if(pwd == NULL) {
                                fprintf(stderr, "ERROR: Unable to obtain user id for %s", dir);
                        } else {
                                printf("%*s", 6, pwd->pw_name);

                        }

                        gid_t groupid = st1.st_gid;
                        struct group *grp;
                        grp = getgrgid(groupid);
                        if(grp == NULL) {
                                fprintf(stderr, "ERROR: Unable to obtain group id for %s", dir);
                        } else {
                                printf("%*s", 9, grp->gr_name); //must alter the number for user and group id with different length
                        }

                        off_t size = st1.st_size;
                        printf("%*lu", 12, size);

                        time_t rawtime;
                        struct tm * timeinfo;
                        time ( &rawtime );
                        timeinfo = localtime( &rawtime );
                        int curYear = timeinfo->tm_year;
                        int curday = timeinfo->tm_yday;


                        struct tm *modificationtime;
                        modificationtime = localtime(&st1.st_mtime);

                        if(!modificationtime) {
                                fprintf(stderr, "ERROR: Could not get mtime for path %s\n", path);
                        }

                        if(!timeinfo) {
                                fprintf(stderr, "WARNING: Could not get current time. Current Time assumed to be in same year as modification time" );
                                timeinfo = modificationtime;
                        }

                        char buf1[1024];
                        if ((modificationtime->tm_year == curYear)) {
                          if (((curday - modificationtime->tm_yday) < 30 )){
                                strftime(buf1, sizeof(buf1), "%b %e %H:%M", modificationtime);
                              }else{
                                strftime(buf1, sizeof(buf1), "%b %e  %Y", modificationtime);
                              }
                        } else {
                                strftime(buf1, sizeof(buf1), "%b %e  %Y", modificationtime);
                        }
                        printf(" %s",buf1);

                        if (issymlink) {
                                char buf[1024];
                                ssize_t len; //typo in man page
                                if ((len = readlink(path, buf, sizeof(buf)-1)) != -1) {
                                        buf[len] = '\0';
                                }else{
                                        fprintf(stderr, "ERROR: Could not read symlink %s: %s\n", path, strerror(errno));
                                        return -1;
                                }
                                strcat(path, " -> ");
                                strcat(path,buf);
                        }
                        printf(" %s\n",path );
                }
                if (!flag) {
                        if (readopendirectory->d_type & DT_DIR) {
                                if (strcmp(directoryname, ".") != 0 && strcmp(directoryname, "..") != 0) {
                                        directorySearch(path, flag);
                                }
                        }
                }else{
                        break;
                }
                errno = 0;
        }
        if(errno) {
          fprintf(stderr, "ERROR: Did not complete reading all the entries for starting directory of %s: %s\n", dir, strerror(errno));
          return -1;
        }
        closedir (opendirectory);

}


int main(int argc, char *argv[]) {
        int flag = 1;
        char* startingdirectory;
        struct stat st;

        switch (argc) {
        case 1:
                startingdirectory = "./";
                break;
        case 2:
                startingdirectory = argv[1];
                break;
        default:
                fprintf(stderr, "ERROR: Invalid input argument specified (Possible errors include invalid amount of arguments specified)");
                return -1;
        }

        if(stat(startingdirectory, &st)<0) {
                fprintf(stderr, "ERROR: Could not get the stat of %s: %s\n",startingdirectory, strerror(errno));
                return -1;
        }

        directorySearch(startingdirectory, flag);
        flag = 0;
        directorySearch(startingdirectory,flag);
        return 0;

}
