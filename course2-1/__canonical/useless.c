#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define log(fmt, ...) fprintf (stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define chk(expr, fmt, ...) do { if (!(expr)) die(fmt, ## __VA_ARGS__); } while (0)

void __attribute__((noreturn)) wait_and_exec(int delay, const char *path)
{
	sleep(delay);
	execlp(path, path, NULL);
	die("Failed to exec() the file \"%s\": %m", path);
}

int main(int argc, char **argv)
{
	int r;

	if (argc != 2) {
		die("This program expects one argument.");
	}

	/*
	 * Open the input.
	 * We use open()+fdopen() sequence instead of just fopen() to set O_CLOEXEC
	 * to avoid leaking the open file into executed processes, and doing
	 * fcntl(fileno(), F_SETFL, fcntl(fileno(), F_GETFL) | O_CLOEXEC) instead is just awkward.
	 */

	const char *in_path = argv[1];
	int in_fd = open(in_path, O_RDONLY | O_CLOEXEC);
	chk(in_fd > 0, "Failed to open() input file \"%s\": %m", in_path);
	FILE *in_file = fdopen(in_fd, "r");
	chk(in_file, "Failed to fdopen() input fd %d: %m", in_fd);

	/*
	 * Ignore SIGCHLD because we don't want to monitor our children.
	 */

	signal(SIGCHLD, SIG_IGN);

	/*
	 * Read the input line-by-line and execute processes.
	 */

	for (;;) {

		int in_delay;
		char *in_path = NULL;

		r = fscanf(in_file, "%d %m[^\n] ", &in_delay, &in_path);
		if (r == EOF) {
			chk(!ferror(in_file), "Failed to read next input line: %m");
			break;
		}
		chk(r == 2, "Failed to parse next input line: parsed %d entries of 2", r);

		int child_pid = fork();
		if (child_pid == 0) {
			/* we are the child */
			wait_and_exec(in_delay, in_path);
		} else if (child_pid < 0) {
			/* we are the parent and fork() had failed */
			die("Failed to fork() off a child: %m");
		}

		/* otherwise go on */
		free(in_path);
	}

	/*
	 * Now wait for all the children.
	 * Since we've ignored the SIGCHLD, wait() shall block and wait for all children
	 * to terminate and _then_ fail with ECHILD.
	 */
	r = wait(NULL);
	assert(r < 0 && errno == ECHILD);

	return 0;
}
