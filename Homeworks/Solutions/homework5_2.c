#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NITEMS		16
#define	NEXT_INDEX(idx)		(((idx) + 1) % NITEMS)

static int buf[NITEMS];
static int read_index, write_index;
static pthread_mutex_t lock;
static pthread_cond_t ready_to_read, ready_to_write; 

static int num_produced;

static bool
buf_full(void)
{
	return (NEXT_INDEX(write_index) == read_index);
}

static bool
buf_empty(void)
{
	return (read_index == write_index);
}

static void *
producer_thread(void *arg)
{
	int thread_number, counter;

	thread_number = (intptr_t)arg;
	pthread_mutex_lock(&lock);
	for (counter = 0; counter < num_produced; counter++) {
		while (buf_full())
			pthread_cond_wait(&ready_to_write, &lock);
		buf[write_index] = thread_number * num_produced + counter;
		write_index = NEXT_INDEX(write_index);
		pthread_cond_signal(&ready_to_read);
	}
	pthread_mutex_unlock(&lock);
	return (NULL);
}

static void *
consumer_thread(void *arg)
{
	int i, items;

	items = (intptr_t)arg;
	pthread_mutex_lock(&lock);
	for (i = 0; i < items; i++) {
		while (buf_empty())
			pthread_cond_wait(&ready_to_read, &lock);
		printf("%d\n", buf[read_index]);
		read_index = NEXT_INDEX(read_index);
		pthread_cond_signal(&ready_to_write);
	}
	pthread_mutex_unlock(&lock);
	return (NULL);
}

int
main(int argc, char **argv)
{
	pthread_t *threads;
	int error, i, nproducers, nconsumers;
	int items_per_consumer, extra_items;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <producers> <consumers> <items>\n",
		    argv[0]);
		return (1);
	}

	nproducers = 1 << atoi(argv[1]);
	nconsumers = 1 << atoi(argv[2]);
	num_produced = 1 << atoi(argv[3]);
	items_per_consumer = num_produced * nproducers / nconsumers;
	extra_items = num_produced * nproducers % nconsumers;
	printf("Using %d producers and %d consumers.\n", nproducers,
	    nconsumers);
	printf("Each producer will generate %d items.\n", num_produced);
	if (extra_items == 0)
		printf("Each consumer will consume %d items.\n",
		    items_per_consumer);
	else {
		printf("%d consumer%s will consume %d items.\n", extra_items,
		    extra_items == 1 ? "" : "s", items_per_consumer + 1);
		printf("%d consumer%s will consume %d items.\n",
		    nconsumers - extra_items, nconsumers - extra_items == 1 ?
		    "" : "s", items_per_consumer);
	}

	error = pthread_mutex_init(&lock, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_mutex_lock");
	}
	error = pthread_cond_init(&ready_to_read, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_cond_init(&ready_to_read)");
	}
	error = pthread_cond_init(&ready_to_write, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_cond_init(&ready_to_write)");
	}

	threads = calloc(nproducers + nconsumers, sizeof(pthread_t));
	for (i = 0; i < nproducers; i++) {
		error = pthread_create(&threads[i], NULL, producer_thread,
		    (void *)(intptr_t)i);
		if (error) {
			errno = error;
			err(1, "pthread_create: producer %d", i);
		}
	}

	for (i = 0; i < nconsumers; i++) {
		error = pthread_create(&threads[nproducers + i], NULL,
		    consumer_thread, (void *)(uintptr_t)(items_per_consumer +
			(i < extra_items ? 1 : 0)));
		if (error) {
			errno = error;
			err(1, "pthread_create: consumer %d", i);
		}
	}

	for (i = 0; i < nproducers + nconsumers; i++) {
		error = pthread_join(threads[i], NULL);
		if (error) {
			errno = error;
			err(1, "pthread_join: thread %d", i);
		}
	}

	pthread_cond_destroy(&ready_to_write);
	pthread_cond_destroy(&ready_to_read);
	pthread_mutex_destroy(&lock);
	printf("Finished\n");
	return (0);
}
