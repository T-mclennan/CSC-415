#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	char buf[1024];
	ssize_t nread, nwritten;
	size_t total;
	int rfd, wfd;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
		return (1);
	}

	rfd = open(argv[1], O_RDONLY);
	if (rfd == -1) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[1],
		    strerror(errno));
		return (1);
	}

	wfd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (wfd == -1) {
		fprintf(stderr, "Failed to create %s: %s\n", argv[2],
		    strerror(errno));
		return (1);
	}

	total = 0;
	for (;;) {
		nread = read(rfd, buf, sizeof(buf));
		if (nread == 0)
			break;
		if (nread == -1) {
			fprintf(stderr, "Read failed: %s\n", strerror(errno));
			return (1);
		}
		nwritten = write(wfd, buf, nread);
		if (nwritten == -1) {
			fprintf(stderr, "Write failed: %s\n", strerror(errno));
			return (1);
		}
		if (nwritten != nread) {
			fprintf(stderr, "Short write: %zd vs %zd\n", nread,
			    nwritten);
			return (1);
		}
		total += nwritten;
	}
	printf("copied %zu bytes\n", total);
	return (0);
}
