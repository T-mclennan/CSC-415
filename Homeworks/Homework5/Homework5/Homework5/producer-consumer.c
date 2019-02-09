//
//  producer-consumer.c
//  Homework5
//
// This program explores the use of a mutex lock for synchronizing the use of multiple threads. It takes in 3
// integers as command line arguments, a count for producers, consumers, and items to be produced. Producers
// create items and store them in a global buffer, consumers take them out and print them to the screen. The
// program terminates when the appropriate amount of items have been produced/consumed.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <pthread.h>
#include <limits.h>

int intBuffer[16];
int writer, reader, to_produce, to_consume, num_produced, bufferCount;
static pthread_cond_t write_ready, read_ready;
static pthread_mutex_t lock;

struct Prod_Info {    // Used as argument to thread_main() for producer threads:
    pthread_t thread_id;
    int       thread_num, count, count_to_produce;
    
};

struct Cons_Info {    // Used as argument to thread_main() for consumer threads:
    pthread_t thread_id;
    int       thread_num, num_consumed, count_to_consume;
    
};

struct Prod_Info Producer_Structs[16];
struct Cons_Info Consumer_Structs[16];


//Function to initialize our queue: sets write and read buffer to start, size = 0:
void InitializeQueue() {
    writer = reader = bufferCount = 0;
}

//Adds item to buffer index referenced by writer, moves writer forward, count++:
void Enqueue(int item) {
    
    if (bufferCount < 16) {
        intBuffer[writer] = item;
        if (writer < 15) {
            writer++;
        } else if (writer == 15) {
            writer = 0;
        }
        bufferCount++;
        pthread_cond_signal(&write_ready);
    }
}

//Returns the item at buffer index referenced by reader, moves reader forward, count--:
int Dequeue () {

    int temp = -1;
    if (bufferCount > 0) {
        temp = intBuffer[reader];
        if (reader < 15) {
            reader++;
        } else if (reader == 15) {
            reader = 0;
        }
        bufferCount--;
        pthread_cond_signal(&read_ready);
    }
        return temp;
}

//Producer_Main:: each producer thread does it's allocation of work:
static void* Producer_Main(void *param)
{
    struct Prod_Info *thread;
    thread = (struct Prod_Info*) param;
    
    //while thread has not reached it's production quota:
    while (thread->count < thread->count_to_produce) {
        

        //if buffer is full, wait
        pthread_mutex_lock(&lock);
        if (writer == reader-1) {
             pthread_cond_wait(&write_ready, &lock);
        }
        
        Enqueue((thread->thread_num*num_produced)+thread->count);
        thread->count++;
        num_produced++;
        pthread_mutex_unlock(&lock);
    }
    
    return param;
}

//Consumer_Main:: each consumer thread does it's allocation of work:
static void* Consumer_Main(void *param)
{
    struct Cons_Info *thread;
    thread = (struct Cons_Info*) param;
    
    //while thread has not reached it's consumption quota:
    while (thread->num_consumed < thread->count_to_consume) {
        
        
        //if buffer is full, wait
        pthread_mutex_lock(&lock);
        if (writer == reader) {
            pthread_cond_wait(&read_ready, &lock);
        }
        
        printf("%d ", Dequeue());
        thread->num_consumed++;
        pthread_mutex_unlock(&lock);
    }
    
    return param;
}

int main(int argc, char * argv[]) {

    int prodCount, consCount, itemCount, error;
    pthread_attr_t attr;
    
    error = pthread_mutex_init(&lock, NULL);
    if (error) {
        errno = error;
        err(1, "pthread_mutex_init");
    }
    
    error = pthread_cond_init(&write_ready, 0);
    if (error) {
        errno = error;
        err(1, "pthread_cond_init_write");
    }
    
    error = pthread_cond_init(&read_ready, 0);
    if (error) {
        errno = error;
        err(1, "pthread_cond_init_read");
    }
    
    //Checks for appropriate number of input arguments:
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <producer count> <consumer count> <item count>\n", argv[0]);
        return (1);
    }
    
    printf("\n");
    
    //Sets attributes for threads:
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Parses command line arguments from a string into ints for further use:
    char *a = argv[1];
    prodCount = 1 << atoi(a);
    a = argv[2];
    consCount = 1 << atoi(a);
    a = argv[3];
    itemCount = 1 << atoi(a);
    
    //Initializes Producers Structs:
    for (int i =0; i < prodCount; i++) {
        
        Producer_Structs[i].thread_num = i;
        Producer_Structs[i].count = 0;
        if (i < prodCount-1) {
                    Producer_Structs[i].count_to_produce = itemCount/prodCount;
        } else if (i == prodCount-1){
                    Producer_Structs[i].count_to_produce = itemCount - (prodCount-1)*(itemCount/prodCount);
        }
    }
    
    //Initializes Consumer Structs:
    for (int i =0; i < consCount; i++) {
        
        Consumer_Structs[i].thread_num = i;
        Consumer_Structs[i].num_consumed = 0;
        if (i < consCount-1) {
            Consumer_Structs[i].count_to_consume = itemCount/consCount;
        } else if (i == consCount-1){
            Consumer_Structs[i].count_to_consume = itemCount - (consCount-1)*(itemCount/consCount);
        }
    }

    InitializeQueue();
    
    //Creates each producer, passing a struct to each with relevant information:
    for (int i = 0; i < prodCount; i++) {
        error = pthread_create(&Producer_Structs[i].thread_id, &attr, Producer_Main,
                               (void*)&Producer_Structs[i]);
        if (error) {
            fprintf(stderr, "Failed to create thread: %s\n",
                    strerror(error));
            return 1;
        }
    }
    
    for (int i = 0; i < consCount; i++) {
        error = pthread_create(&Consumer_Structs[i].thread_id, &attr, Consumer_Main,
                               (void*)&Consumer_Structs[i]);
        if (error) {
            fprintf(stderr, "Failed to create thread: %s\n",
                    strerror(error));
            return 1;
        }
    }
    //Main thread waits for each producer thread to finish:
    for (int i =0; i < (prodCount); i++) {
        error = pthread_join(Producer_Structs[i].thread_id, NULL);
        if (error) {
            fprintf(stderr,
                    "Failed to join thread: %s\n",
                    strerror(error));
            return 1;
            
        }
    }
    
    //Main thread waits for each consumer thread to finish:
    for (int i =0; i < (consCount); i++) {
        error = pthread_join(Consumer_Structs[i].thread_id, NULL);
        if (error) {
            fprintf(stderr,
                    "Failed to join thread: %s\n",
                    strerror(error));
            return 1;
            
        }
    }
    printf("\n\nProgram finished. Producer count: %d | Consumer Count: %d | Item count: %d\n\n", prodCount, consCount, itemCount);
    
    pthread_cond_destroy(&read_ready);
    pthread_cond_destroy(&write_ready);
    pthread_mutex_destroy(&lock);
    return 0;
}
