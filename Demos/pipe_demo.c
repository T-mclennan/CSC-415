#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	int fd[2];

	pipe(fd);
	if (fork() == 0) {
		int x;

		close(fd[1]);
		while (read(fd[0], &x, sizeof(x)) == sizeof(x))
			printf("child: got %d\n", x);
		printf("child: exiting\n");
		exit(0);
	} else {
		int y;

		close(fd[0]);
		while (scanf("%d", &y) == 1)
			write(fd[1], &y, sizeof(y));
		printf("parent: exiting\n");
		return (0);
	}
}
