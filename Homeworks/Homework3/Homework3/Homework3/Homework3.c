//  Homework3.c
//  CSC 415 (01) - Prof. Baldwin
//  Tristan Mclennan - 3/1/18.
//
//  The program serves as a basic command shell, that runs on top of the linux/unix terminal. It takes a maximum of 5 arguments (1024 bytes), the first is the command to be run, and then up to 4 other options/arguments can be accommodated. The input of "exit" will terminate the program.

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#define INPUT_SIZE 1024

int main(int argc, const char * argv[]) {

    
    char inputBuff[INPUT_SIZE];
    char s[5] = " \t\n";
    char* tokensArray[6];
    char* token;
    int runShell = 1;

    
    while (runShell) {
        
        //fresh input count:
        int inputCount = 0;
        for (int i=0; i <6; i++) {
            tokensArray[i] = NULL;
        }
        
        printf("> ");
        fgets(inputBuff, INPUT_SIZE, stdin);
        
        //checks first input argument:
        token = strtok(inputBuff, s);
        
        if (token != NULL) {
            tokensArray[inputCount] = token;
            inputCount++;
            
        } else
            printf("Error: No input. Command structure: <command> <arg1> <arg2> <arg3> <arg4>\n");
        
        // Checks for the exit condition, short circuiting if the input was empty:
        if (token != NULL && !strcmp(token, "exit")) {
            runShell = 0;
            break;
        }
        
        //Tokenizing the input:
        while (token != NULL) {
            if (inputCount > 5) {
                printf("Error: too many arguments given. Ex: <command> <arg1> <arg2> <arg3> <arg4>\n" );
                break;
            }
            
            token = strtok(NULL, s);
            tokensArray[inputCount] = token;
            if (token != NULL) {
                inputCount++;
            }
        }
        if (inputCount<6) {
            
            //Forks current process, uses execvp to turn child into process defined by user input:
            if (fork() == 0) {
                execvp(tokensArray[0], tokensArray);
                perror("Error: exec failed. Ex: <command> <arg1> <arg2> <arg3> <arg4>\n" );
                exit(1);
            
            } else {
                wait(NULL);
            }
        }
    }
    
return 0;
}
