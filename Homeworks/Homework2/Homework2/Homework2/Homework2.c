
//  Homework2.c
//  CSC 415 (01) - Prof. Baldwin
//  Tristan Mclennan - 2/14/18.
//
//  The function of this program is similar to the cp in UNIX, which serves to copy the contents of a given file into a new one. It takes two command-line arguments, the name of the old file and the name of the new file. It returns 0 if the copy is successful.
    
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//argv[1] = the first argument, the original filename.
//argv[2] = the second argument, the name of the new file.
int main(int argc, const char * argv[]) {
    
    char buff[1024];
    ssize_t nread, nwritten, ntotal=0;
    
    int flag = 1;
    int fd[2];
    
    // checks to make sure we have 2 arguments:
    if (argc != 3) {
        printf("Usage: ./homework2 <source> <destination> \n");
        exit(1);
    } else {
    
        // opens source file:
        fd[0] = open(argv[1], O_RDONLY);
        if (fd[0] == -1) {
            printf("error opening file: %s\n", strerror(errno));
            exit(1);
        }
        
        // opens destination file:
        fd[1] = open(argv[2], O_WRONLY | O_CREAT | O_EXCL);
        if (fd[1] == -1) {
            printf("error opening file: %s\n", strerror(errno));
            exit(1);
        }
    
    // Loops through the source file, copying up to one buffer volume (1024) at a time, and writing to destination file:
        
    while (flag == 1) {
            nread = read(fd[0], buff, 1024);
            if (nread == 0) {
                printf("error reading file: %s\n", strerror(errno));
                exit(1);
        }
    
        nwritten = write(fd[1], buff, nread);
        if (nwritten == 0) {
            printf("error writing file: %s\n", strerror(errno));
            exit(1);
        }
    
                if (nread < 1024) {
                    flag = 0;
                }
            ntotal += nwritten;
    }
        
        //close files at end of execution:
        if (close(fd[0]) != 0) {
            printf("error closing file: %s\n", strerror(errno));
            exit(1);
        }
        
        if (close(fd[1]) != 0) {
            printf("error closing file: %s\n", strerror(errno));
            exit(1);
        }
        
        printf("file copied. %zd bytes written.\n", ntotal);

        return 0;
        
    }
}

