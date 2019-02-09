//
//  simulate-paging.c
//  Homework 6  -  Tristan Mclennan
//
//  This program aims to simulate FIFO and LRU algorithms for virtual memory management.
//  It takes in 4 command line arguments: an int <page size>, int <virtual memory size>
//  a string <FIFO/LRU>, and int <number frames>. It simulates the chosen page replacement
//  algorithm and outputs the total number of page faults at the end.

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

// MAIN //
int main(int argc, const char * argv[]) {

    int pageSize, numFrames, numPages, input;
    char temp[20];
    char *endptr;
    int count = 0, pageFaults =0;
    int nextFrame =0;

    
    //checks the input for argument count:
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <page size> <total virtual memory> <FIFO/LRU><# frames>\n", argv[0]);
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
    
    //If FIFO is chosen:
    if (!strcmp(argv[3], "FIFO")) {
        
        //reads and parses a line from stdin, uses bitshift to remove offset:
        while(fgets(temp, 20, stdin) != NULL){
    
            input = (int)strtol(temp, &endptr, 16);
            input = input >> atoi(argv[1]);
            
            //If page doesn't exist: record fault, put entry into table and flip valid bit,
            //invalidate bit from data previously stored in frameTable entry, move
            //forward the counter for next frame to be used in frameTable.
            if (pageTable[input].valid == 0) {
                count++;
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
            }
        }
        
        //If fgets() has an error, print message and exit:
         if (!feof(stdin)) {
         fprintf(stderr, "Failed to read: %s\n", strerror(errno));
         return (1);
         }
        
    } else if (!strcmp(argv[3], "LRU")) {

        //reads and parses a line from stdin, uses bit shifting to remove offset:
        while(fgets(temp, 20, stdin) != NULL){
            input = (int)strtol(temp, &endptr, 16);
            input = input >> atoi(argv[1]);
            
            //If page doesn't exist: record fault, put entry into table and flip valid bit,
            //invalidate bit from data previously stored in frameTable entry, if frameTable
            //is full this will use the time counter to replace the oldest entry.
            if (pageTable[input].valid == 0) {
                if (count < numFrames) {
                    count++;
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
        }
        
        //If fgets() has an error, print message and exit:
        if (!feof(stdin)) {
            fprintf(stderr, "Failed to read: %s\n", strerror(errno));
            return (1);
        }
        
    } else {
        printf("Algorithm %s not recognized. Please try 'LRU' or 'FIFO'.\n", argv[3]);
    }
    
    printf("Page faults: %d\n", pageFaults);
    return 0;
}
