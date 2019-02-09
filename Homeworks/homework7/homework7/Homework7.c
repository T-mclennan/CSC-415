//
//  Homework7.c
//  CSC 415 (01) - Prof. Baldwin
//  Tristan Mclennan - 3/1/18.
//
//  The program serves as a basic command shell, that runs on top of the linux/unix terminal. It takes a maximum
//  of 5 arguments (1024 bytes), the first is the command to be run, and then up to 4 other options/arguments
//  can be accommodated. The input of "exit" will terminate the program. Program accepts <, >, 2>, >>, 2>>, |,
//  and & operators. I had a huge amount of trouble getting pipe to work, and through testing kept making things
//  more simple and redundant. I would love to polish it up more, but unfortunately I am already at the deadline, so I have to submit what I have. :( Thanks!

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <stdbool.h>


int globalargc = 0;

static bool
split_line(char *line, char **argv, int max_args)
{
    char *word;
    int words;
    
    words = 0;
    word = strtok(line, " \t\n");
    while (word != NULL) {
        if (words >= max_args) {
            printf("Too many arguments\n");
            return (false);
        }
        argv[words] = word;
        words++;
        word = strtok(NULL, " \t\n");
    }
    argv[words] = NULL;
    globalargc = words;
    return (true);
}

int
main(void)
{
    char *argv[6];
    char line[1024];
    pid_t pid, mypid= 0;
    int fdArray[3], pipeFd[2];
    int tokenCount = 0;
    int rhandinput=0, pipeCount = 0;
    bool isPipe = false;
    bool isBackground = false;


    
    for (;;) {
        
        //Basic shell prompt:
        printf("cShell> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin))
                return (0);
            fprintf(stderr, "Failed to read command: %s\n",
                    strerror(errno));
            return (1);
        }
        if (!split_line(line, argv, 5))
            continue;
        if (argv[0] == NULL) {
            printf("Empty command\n");
            continue;
        }
        if (strcmp(argv[0], "exit") == 0)
            return (0);
        
        char* tokens[globalargc+1];

        
        // Prepares token buffer:
        for (int i =0; i< 6; i++) {
            tokens[i] = NULL;
        }
        
        // Sets file descripter information to empty:
        isPipe = false;
        for (int i =0; i < 3; i++) {
            fdArray[i] = -1;
            pipeFd[i] = -1;
        }

        
        //Loops through parsing input, searching for operators and preparing arguments to pass to fork():
        for (int i = 0; i < globalargc; i++) {
            
            
            if (strcmp(argv[i], "&") == 0) {
                
                //if & is the last input, flag background as true:
                if (i == (globalargc-1)) {
                    isBackground = true;
                } else {
                    printf("Error. Proper usage is: <input> <input> &\n");
                    return(1);
                }
                break;
                
                // Input operator redirects stdin:
            } else if (strcmp(argv[i], "<") == 0) {
                
                fdArray[0] = open(argv[i+1], O_RDONLY);
                
                //error check on open():
                if (fdArray[0] == -1)
                    fprintf(stderr, "Failed to open: %s\n", strerror(errno));
                break;
                
                // redirect operator - changes stdout descriptor to fit new FD:
            } else if (strcmp(argv[i], ">") == 0) {
                
                fdArray[1] = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
                
                //error check on open():
                if (fdArray[1] == -1) {
                    fprintf(stderr, "Failed to open: %s\n", strerror(errno));
                }
    
                break;
                
            
                // redirect operator from stderror:
            } else if (strcmp(argv[i], "2>") == 0) {
                fdArray[2] = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0777);
                
                //error check on open():
                if (fdArray[2] == -1)
                    fprintf(stderr, "Failed to open: %s\n", strerror(errno));
                
                break;
                
                //appending operator from stdin:
            } else if (strcmp(argv[i], ">>") == 0) {
                fdArray[1] = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0777);
                
                //error check on open():
                if (fdArray[1] == -1) {
                    fprintf(stderr, "Failed to open: %s\n", strerror(errno));
                }
                break;
                
                //appending operator from stderror:
            } else if (strcmp(argv[i], "2>>") == 0) {
                fdArray[2] = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0777);
                
                //error check on open():
                if (fdArray[2] == -1)
                    fprintf(stderr, "Failed to open: %s\n", strerror(errno));
                
                break;
                
                //Pipe operator - routes output from operand 1 into input of operand 2:
            } else if (strcmp(argv[i], "|") == 0) {
                isPipe = true;
                pipeCount = i;
                
                // error handling for pipe:
                if (pipe(pipeFd) == -1) {
                    fprintf(stderr, "Failed to create pipe: %s\n",
                            strerror(errno));
                   
                }
                
                rhandinput = pipeFd[0];
                fdArray[1] = pipeFd[1];
                
                
            } else {
                    
                tokens[tokenCount] = argv[i];
                tokenCount++;
                }
            
        }  // end parse loop
        
        //sets last token in array to NULL:
        tokens[tokenCount] = NULL;

        if (isPipe) {
            
            fdArray[1] = pipeFd[1];
            //With fork produces a child process for R side of pipe:
            pid = fork();
            
            //Error checking for fork:
            if (pid == -1) {
                fprintf(stderr, "Failed to fork: %s\n",
                        strerror(errno));
                return (1);
            }
            
            if (pid == 0) {
                
                for (int i = 0; i < (globalargc-pipeCount); i++) {
                    tokens[(pipeCount)+i] = NULL;
                }
                
          //      fdArray[1] = pipeFd[1];
                
                //Uses dup2 to set desciptors for first operand:
                for (int i = 0; i < 3; i++) {
                    if (fdArray[i] != -1) {
                        if (dup2(fdArray[i], i) == -1) {
                            printf("Dup2 failed to copy descriptors: %s\n", strerror(errno));
                            exit(1);
                        }
                        close(fdArray[i]);
                        printf(" %d ", fdArray[i]);
                    }
                }
                
                //close read descriptor of pipe
                if (rhandinput != -1){
                     close(rhandinput);
                }

                //File descriptors carry over after exec:
                execvp(tokens[0], tokens);
                fprintf(stderr, "Failed to execute %s: %s\n",
                        argv[0], strerror(errno));
                exit(1);
                
                
            }
            
        }
        

        //Fork produces first child process:
        pid = fork();
        
        //Error checking for fork:
        if (pid == -1) {
            fprintf(stderr, "Failed to fork: %s\n",
                    strerror(errno));
            return (1);
        }
        
        if (pid == 0) {
            
            if (isPipe) {
                 fdArray[0] = rhandinput;
                 fdArray[1] = -1;
            }

            //Uses dup2 to set desciptors for second operand:
            for (int i = 0; i < 3; i++) {
                if (fdArray[i] != -1) {
                    if (dup2(fdArray[i], i) == -1) {
                        printf("Dup2 failed to copy descriptors: %s\n", strerror(errno));
                        exit(1);
                    }
                    close(fdArray[i]);
                }
            }
            
            //if a pipe is present, exec the operand from the right side of the pipe first:
            if (isPipe) {
                
                for (int i = 0; i < (globalargc-pipeCount); i++) {
                    tokens[0+i] = tokens[(pipeCount)+i];
                    tokens[(pipeCount)+i] = NULL;
                }
                
                //closes write descriptor of pipe for second operand:
                close(pipeFd[1]);
                
            }
            
            //File descriptors carry over after exec:
            execvp(tokens[0], tokens);
            fprintf(stderr, "Failed to execute %s: %s\n",
                    argv[0], strerror(errno));
            exit(1);
        }
        
        //cleanup: close and reset file descriptors, reset arg array:
        tokenCount = 0;
        globalargc = 0;
        for (int i = 0; i < 3; i++)
            if (fdArray[i] != -1) {
                close(fdArray[i]);
            }
        
        for (int i = 0; i < 3; i++) {
            fdArray[i] = -1;
        }
        
        //Close pipes if necessary:
        if (isPipe) {
            close(pipeFd[0]);
            close(pipeFd[1]);
        }

        
        //if not a background process, wait for the children:
        if (!isBackground){
            while (mypid != pid)
                mypid = wait(NULL);
                if (mypid == -1)
                    fprintf(stderr, "Error on wait: %s\n", strerror(errno));
        }

    }
}

