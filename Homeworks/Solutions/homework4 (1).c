#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#ifndef NTHREADS
#define	NTHREADS 4
#endif

static char buf[64 * 1024 * 1024];
static int counts[NTHREADS][128];

struct thread_info {
	int start;
	int length;
	int index;
};

static void *
counter_thread(void *arg)
{
	struct thread_info *info;
	char *p;
	unsigned int c;

	info = arg;
	for (p = buf + info->start; info->length; info->length--, p++) {
		c = *p;
		if (c < 128)
			counts[info->index][c]++;
	}
	return (NULL);
}

static void
print_sums(void)
{
	int i, j, sum;

	for (i = 0; i < 128; i++) {
		sum = 0;
		for (j = 0; j < NTHREADS; j++)
			sum += counts[j][i];
		printf("%d occurrence%s of ", sum, sum == 1 ? "" : "s");
		if (isprint(i) && !isspace(i))
			printf("'%c'\n", i);
		else
			printf("0x%x\n", i);
	}
}

int
main(int argc, char **argv)
{
	pthread_t threads[NTHREADS];
	struct thread_info info[NTHREADS];
	ssize_t nread;
	int error, fd, i, start;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <ASCII file>\n", argv[0]);
		return (1);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		err(1, "open(%s)", argv[1]);
	nread = read(fd, buf, sizeof(buf));
	if (nread == -1)
		err(1, "read");
	close(fd);

	start = 0;
	for (i = 0; i < NTHREADS - 1; i++) {
		info[i].start = start;
		info[i].length = nread / NTHREADS;
		start += info[i].length;
		info[i].index = i;
	}
	info[NTHREADS - 1].start = start;
	info[NTHREADS - 1].length = nread - start;
	info[NTHREADS - 1].index = NTHREADS - 1;

	for (i = 0; i < NTHREADS; i++) {
		error = pthread_create(&threads[i], NULL, counter_thread,
		    &info[i]);
		if (error) {
			errno = error;
			err(1, "pthread_create(%d)", i);
		}
	}

	for (i = 0; i < NTHREADS; i++) {
		error = pthread_join(threads[i], NULL);
		if (error) {
			errno = error;
			err(1, "pthread_join(%d)", i);
		}
	}

	print_sums();
	return (0);
}
