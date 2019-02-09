#include <pthread.h>
#include <stdio.h>
#include <string.h>

static void *
thread_main(void *arg)
{
	const char *name = arg;

	printf("thread %s: id %p\n", name, pthread_self());
#if 0
	/*
	 * Enable this to force the program to wait with both threads
	 * active.  This allows one to use tools like ps(1) to see
	 * the backing kernel threads.
	 */
	getchar();
#endif
	pthread_exit(NULL);
}

int
main(void)
{
	pthread_t thr;
	int error;

	error = pthread_create(&thr, NULL, thread_main, "child");
	if (error) {
		fprintf(stderr, "Failed to create thread: %s\n",
		    strerror(error));
		return 1;
	}

	printf("main thread: id %p started %p\n", pthread_self(), thr);
	error = pthread_join(thr, NULL);
	if (error) {
		fprintf(stderr, "Failed to join thread: %s\n",
		    strerror(error));
		return 1;
	}

	return 0;
}
