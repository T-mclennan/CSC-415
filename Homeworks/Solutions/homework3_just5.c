#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool
split_line(char *line, char **argv, int max_args)
{
	char *word;
	int words;

	words = 0;
	word = strtok(line, " \t\n");
	while (word != NULL) {
		if (words >= max_args) {
			printf("Too many arguments\n");
			return (false);
		}
		argv[words] = word;
		words++;
		word = strtok(NULL, " \t\n");
	}
	argv[words] = NULL;
	return (true);
}

int
main(void)
{
	char *argv[6];
	char line[1024];
	pid_t pid;

	for (;;) {
		printf("> ");
		if (fgets(line, sizeof(line), stdin) == NULL) {
			if (feof(stdin))
				return (0);
			fprintf(stderr, "Failed to read command: %s\n",
			    strerror(errno));
			return (1);
		}
		if (!split_line(line, argv, 5))
			continue;
		if (argv[0] == NULL) {
			printf("Empty command\n");
			continue;
		}
		if (strcmp(argv[0], "exit") == 0)
			return (0);
		pid = fork();
		if (pid == 0) {
			execvp(argv[0], argv);
			fprintf(stderr, "Failed to execute %s: %s\n",
			    argv[0], strerror(errno));
			exit(1);
		}
		if (pid == -1) {
			fprintf(stderr, "Failed to fork: %s\n",
			    strerror(errno));
			return (1);
		}

		wait(NULL);
	}
}
