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
#ifdef WAIT
		int child_status;
		long r = waitpid(child, &child_status, 0);
		if (r < 1) {
			fprintf(stderr, "Failed to wait() for pid %ld: %m\n", (long)child);
			return 1; /* we cannot guarantee ordering anymore */
		} else if (r != child) {
			fprintf(stderr, "Unexpected return from wait() for pid %ld: %ld\n", (long)child, (long)r);
			return 1; /* we cannot guarantee ordering anymore */
		} else {
			if(!WIFEXITED(child_status) || (WEXITSTATUS(child_status) != 0)) {
				fprintf(stderr, "child #%zu (pid %ld) failed somewhy\n", i + 1, (long)child);
			}
			/* otherwise it's ok */
		}
#endif
	}

	return fork_failed ? 1 : 0;
}
