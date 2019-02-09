#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t lock;
static pthread_cond_t data_ready, thread_idle;

/*
 * x < 0  => thread requested to exit
 * x == 0 => nothing to do
 * x > 0  => output factorial of x
 */
static int x;

static int
factorial(int x)
{
	if (x == 1)
		return 1;
	return x * factorial(x - 1);
}

static void *
thread_main(void *arg)
{

	pthread_mutex_lock(&lock);
	while (x >= 0) {
		if (x == 0)
			pthread_cond_wait(&data_ready, &lock);
		else {
			printf("%d! is %d\n", x, factorial(x));
			x = 0;
			pthread_cond_signal(&thread_idle);
		}
	}
	pthread_mutex_unlock(&lock);
	return (NULL);
}

int
main(void)
{
	pthread_t thread;
	int error, y;

	error = pthread_mutex_init(&lock, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_mutex_init");
	}

	error = pthread_cond_init(&data_ready, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_cond_init(data_ready)");
	}

	error = pthread_cond_init(&thread_idle, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_cond_init(thread_idle)");
	}

	error = pthread_create(&thread, NULL, thread_main, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_create");
	}

	while (scanf("%d", &y) == 1) {
		if (y <= 0) {
			printf("Invalid value: %d\n", y);
			continue;
		}
		pthread_mutex_lock(&lock);
		while (x != 0)
			pthread_cond_wait(&thread_idle, &lock);
		x = y;
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&data_ready);
	}

	pthread_mutex_lock(&lock);
	while (x != 0)
		pthread_cond_wait(&thread_idle, &lock);
	x = -1;
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&data_ready);

	error = pthread_join(thread, NULL);
	if (error) {
		errno = error;
		err(1, "pthread_join");
	}

	pthread_cond_destroy(&thread_idle);
	pthread_cond_destroy(&data_ready);
	pthread_mutex_destroy(&lock);
	return (0);
}
