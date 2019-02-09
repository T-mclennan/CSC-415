#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(void)
{
	pid_t pid;

	printf("%d: I'm the parent\n", getpid());
	pid = fork();
	if (pid == 0) {
		printf("%d: I'm the child\n", getpid());
		exit(0);
	}

	(void)getchar();
	pid = wait(NULL);
	printf("Child %d exited\n", pid);
	return 0;
}
