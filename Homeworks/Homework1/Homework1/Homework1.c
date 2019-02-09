//  Homework1.c
//  CSC 415 (01) - Prof. Baldwin
//  Tristan Mclennan - 1/31/18.
//
//

#include <stdio.h>
#include <unistd.h>

#define NAME "Tristan"

int main() {

    size_t buff_size = 30;
    char myName[buff_size];
    
    snprintf(myName, buff_size, "Hello 415! I am %s.\n", NAME);
    write(1, myName, buff_size);
    
    
    return 0;
}
