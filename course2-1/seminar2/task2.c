#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

long parse_integer(const char *arg)
{
	char *endptr = NULL;
	errno = 0;
	long value = strtol(arg, &endptr, 10);

	if (*arg == '\0' || *endptr != '\0') {
		fprintf (stderr, "Argument is not a valid decimal number: '%s'\n", arg);
		exit(1);
	}

	if (errno) {
		fprintf (stderr, "Argument parse error: '%s': %m\n", arg);
		exit(1);
	}

	return value;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "This program expects one argument.\n");
		return 1;
	}

	int fork_failed = 0;
	long children_count = parse_integer(argv[1]);

	for (long i = 0; i < children_count; ++i) {
		long child = fork();

		if (child < 0) {
			/* parent, fork() failed */
			fprintf(stderr, "Failed to fork() child #%zu: %m\n", i + 1);
			fork_failed = 1;
		} else if (child == 0) {
			/* child */
			printf("child #%zu pid %ld ppid %ld\n", i + 1, (long)getpid(), (long)getppid());
			return 0;
		}

		/* parent, fork() succeeded */
	}

	return fork_failed ? 1 : 0;
}
