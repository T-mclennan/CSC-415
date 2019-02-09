#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NAME "John Baldwin"

int
main(int argc, char *argv[])
{
	char buf[64];

	snprintf(buf, sizeof(buf), "Hello 415, I am %s!\n", NAME);
	write(1, buf, strlen(buf));
	return (0);
}
