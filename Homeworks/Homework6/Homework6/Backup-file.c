//
//  simulate-paging.c
//  Homework 6  -  Tristan Mclennan
//
//  This program aims to simulate FIFO and LRU algorithms for virtual memory management.
//



#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

//pageEntry structs store physical address of data, plus valid bit, time counter:
struct pageEntry {
    int entry;
    int valid;
    int time;
};

int main(int argc, const char * argv[]) {
    
    int pageSize, numFrames, numPages, input;
    char temp[20];
    char *endptr;
    int count = 0, pageFaults =0;
    int nextFrame =0;
    
    
    //checks the input for argument count:
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <page size> <total virtual memory> <# frames>\n", argv[0]);
        return (1);
    }
    
    //converts command line args into appropriate variables:
    pageSize   = 1 << atoi(argv[1]);
    numFrames  = atoi(argv[4]);
    numPages = (1 << atoi(argv[2])) / pageSize;
    
    //pageTable array holds pageEntry structs, bit set invalid, time counter set to 0:
    struct pageEntry pageTable[numPages];
    for (int i =0; i < numPages; i++) {
        pageTable[i].valid = 0;
        pageTable[i].entry = 0;
        pageTable[i].time = 0;
    }
    
    int frameTable[numFrames];
    for (int i =0; i < numFrames; i++) {
        frameTable[i] = 0;
    }
    
    if (!strcmp(argv[3], "FIFO")) {
        printf("This is FIFO! ");
        
        //reads and parses a line from stdin:
        while(fgets(temp, 20, stdin) != NULL){
            
            /*  //If fgets() has an error, print message and exit:
             if (!feof(stdin)) {
             fprintf(stderr, "Failed to read: %s\n", strerror(errno));
             return (1);
             }  */
            
            printf("This is input: %s | ", temp);
            input = (int)strtol(temp, &endptr, 16);
            input = input >> atoi(argv[1]);
            printf("Page: %d | ", input);
            printf(" nextframe: %d\n", nextFrame);
            
            //If page doesn't exist: record fault, put entry into table and flip valid bit,
            //invalidate bit from data previously stored in frameTable entry, move
            //forward the counter for next frame to be used in frameTable.
            if (pageTable[input].valid == 0) {
                count++;
                printf("page fault: %d\n", input);
                if (count > numFrames){
                    pageTable[frameTable[nextFrame]].valid = 0;
                }
                pageTable[input].entry = nextFrame;
                pageTable[input].valid = 1;
                frameTable[nextFrame] = input;
                pageFaults++;
                if (nextFrame < (numFrames -1)) {
                    nextFrame++;
                } else
                    nextFrame = 0;
                
            } else {
                printf("page hit: %d\n", input);
            }
            
            for (int i =0; i < numPages; i++) {
                printf("%d-%d ", pageTable[i].entry, pageTable[i].valid);
            }
            
            printf("| ");
            
            for (int i =0; i < numFrames; i++) {
                printf("%d ", frameTable[i]);
            }
            
            printf("\n");
        }
        
    } else if (!strcmp(argv[3], "LRU")) {
        printf("This is LRU! ");
        
        //reads and parses a line from stdin:
        while(fgets(temp, 20, stdin) != NULL){
            
            printf("This is input: %s | ", temp);
            input = (int)strtol(temp, &endptr, 16);
            //printf("Input: %d | ", input);
            input = input >> atoi(argv[1]);
            printf("Page: %d | ", input);
            printf(" nextframe: %d\n", nextFrame);
            
            //If page doesn't exist: record fault, put entry into table and flip valid bit,
            //invalidate bit from data previously stored in frameTable entry, move
            //forward the counter for next frame to be used in frameTable.
            if (pageTable[input].valid == 0) {
                if (count < numFrames) {
                    count++;
                    printf("page fault: %d\n", input);
                    pageTable[input].entry = nextFrame;
                    pageTable[input].valid = 1;
                    pageTable[input].time = 0;
                    frameTable[nextFrame] = input;
                    pageFaults++;
                    nextFrame++;
                    
                    //If frameTable is full, use time counter to choose oldest entry to replace:
                } else {
                    
                    nextFrame = 0;
                    for (int i=0; i < numFrames; i++) {
                        if (pageTable[frameTable[i]].time > pageTable[frameTable[nextFrame]].time) {
                            nextFrame = i;
                        }
                    }
                    
                    pageTable[frameTable[nextFrame]].valid = 0;
                    pageTable[input].entry = nextFrame;
                    pageTable[input].valid = 1;
                    pageTable[input].time = 0;
                    frameTable[nextFrame] = input;
                    pageFaults++;
                    
                }
                
                
                //resets time counter on hit, moves all time forward:
            } else {
                printf("page hit: %d\n", input);
                pageTable[input].time = 0;
            }
            
            //Moves forward the time counter for all valid entries in frameTable:
            if (count < numFrames) {
                for (int i=0; i < numPages; i++){
                    if (pageTable[i].valid == 1) {
                        pageTable[i].time++;
                    }
                }
            } else {
                for (int i=0; i < numFrames; i++) {
                    if (pageTable[frameTable[i]].valid == 1) {
                        pageTable[frameTable[i]].time++;
                    }
                }
            }
            
            for (int i =0; i < numPages; i++) {
                printf("%d-%d-%d ", pageTable[i].entry, pageTable[i].time, pageTable[i].valid);
            }
            
            printf("| ");
            
            for (int i =0; i < numFrames; i++) {
                printf("%d ", frameTable[i]);
            }
            
            printf("\n");
        }
        
    } else {
        
        printf("Algorithm %s not recognized. Please try 'LRU' or 'FIFO'.\n", argv[3]);
    }
    
    
    printf("PageSize: %d, TotalVM: %d, Algo: %s, Frames: %d\n", pageSize, 1 << atoi(argv[2]), argv[3], numFrames);
    
    printf("Page faults: %d\n", pageFaults);
    
    
    
    return 0;
}

