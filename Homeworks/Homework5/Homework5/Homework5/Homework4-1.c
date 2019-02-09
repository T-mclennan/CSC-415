//
//  Homework4-part2.c
//  CSC 415 (01) - Prof. Baldwin
//  Tristan Mclennan - 3/1/18.
//
//  This program serves as a point of analysis for the computer resources used by multi-threaded processing.
//  It takes the address of a text file, and returns the frequency count of each character, but it does so
//  through using a pre-defined number of threads. With this information it is possible to assess the relative
//  performance of the application with different thread counts.

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <pthread.h>

#define THREAD_COUNT 4

char char_buffer[67108864];
//int countBuffer[THREAD_COUNT+1][128];
int countBuffer[128];
size_t file_size, alloc;
int *ret;

struct Thread_Info {    /* Used as argument to thread_main() */
    pthread_t thread_id;
    int       thread_num;
    unsigned long startIndex;
    unsigned long numToRead;
    
};

struct Thread_Info Structs[THREAD_COUNT];

//Thread_Main:: each thread does it's allocation of work:
static void* thread_main(void *param) //struct Thread_Info thread)
{
    struct Thread_Info *thread;
    thread = (struct Thread_Info*) param;
    int c;
    
    for (int i = 0; i < thread->numToRead; i++ ) {
        c = (int)char_buffer[thread->startIndex+i];
        //countBuffer[thread->thread_num][c]++;
        countBuffer[c]++;
    }
    return param;
}

//::MAIN::
int main(int argc, const char * argv[]) {
    
    pthread_attr_t attr;
    ssize_t nwritten = 0;
    FILE * file;
    int error = 0;
    
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    
    //Checks for appropriate number of input arguments:
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return (1);
    }
    
    //Opens source file:
    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("error opening file: %s\n", strerror(errno));
        exit(1);
    }
    
    //Reads from the file, writes into buffer:
    nwritten = fread(char_buffer, sizeof(char), 67108864, file);
    
    //alloc is the portion of the buffer each thread will work on:
    if (!THREAD_COUNT) {
        printf("Thread count is zero. Exiting:\n");
        exit(1);
    } else {
        alloc = nwritten / THREAD_COUNT;
        
    }
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        
        //Stores thread information in Struct Array:
        Structs[i].startIndex = i*alloc;
        if (i+1 == THREAD_COUNT) {
            Structs[i].numToRead = nwritten - (i)*alloc;
        } else {
            Structs[i].numToRead = alloc;
        }
        Structs[i].thread_num = i;
    }
    
    //Creates each thread, passing a struct to each with relevant information:
    for (int i = 0; i < THREAD_COUNT; i++) {
        error = pthread_create(&Structs[i].thread_id, &attr, thread_main,
                               (void*)&Structs[i]);
        if (error) {
            fprintf(stderr, "Failed to create thread: %s\n",
                    strerror(error));
            return 1;
        }
    }
    
    //Main thread waits for each child thread to finish:
    for (int i =0; i < THREAD_COUNT; i++) {
        error = pthread_join(Structs[i].thread_id, NULL);
        if (error) {
            fprintf(stderr,
                    "Failed to join thread: %s\n",
                    strerror(error));
            return 1;
            
        }
    }
    
    //Consolidates thread counts into master array, array[THREAD_COUNT+1]:
    for (int i =0; i < 128; i++) {
       /* for (int j = 0; j < THREAD_COUNT; j++) {
            countBuffer[THREAD_COUNT+1][i] += countBuffer[j][i];
        }*/
        
        if (i < 32) {
           // printf("%d occurances of 0x%X\n", countBuffer[THREAD_COUNT+1][i], i);
            printf("%d occurances of 0x%X\n", countBuffer[i], i);
        } else if (i == 127){
            //printf("%d occurances of 'DEL'\n", countBuffer[THREAD_COUNT+1][i]);
            printf("%d occurances of 'DEL'\n", countBuffer[i]);
        } else {
            char c = i;
            //printf("%d occurances of '%c'\n", countBuffer[THREAD_COUNT+1][i], c);
            printf("%d occurances of '%c'\n", countBuffer[i], c);
        }
    }
    
    fclose(file);
    return 0;
}

