#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static const size_t CHILDREN_COUNT = 10;

int main(int argc, char **argv)
{
	if (argc != 1) {
		fprintf(stderr, "This program expects no arguments.\n");
		return 1;
	}

	int fork_failed = 0;

	for (size_t i = 0; i < CHILDREN_COUNT; ++i) {
		long child = fork();

		if (child < 0) {
			/* parent, fork() failed */
			fprintf(stderr, "Failed to fork() child #%zu: %m\n", i + 1);
			fork_failed = 1;
			continue;
		} else if (child == 0) {
			/* child */
			printf("child #%zu pid %ld ppid %ld\n", i + 1, (long)getpid(), (long)getppid());
			return 0;
		}

		/* parent, fork() succeeded */
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
	}

	return fork_failed ? 1 : 0;
}
