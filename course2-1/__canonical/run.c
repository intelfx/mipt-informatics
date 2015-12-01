#include "../common.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	int r;

	if (argc != 2) {
		die_ret("This program expects one argument.");
	}

	if (strempty(argv[1])) {
		die_ret("Empty number of tasks passed.");
	}

	errno = 0;
	char *endptr = NULL;
	unsigned long tasks = strtoul(argv[1], &endptr, 10);
	if (!strempty(endptr) || errno != 0) {
		die_ret("Invalid number of tasks \"%s\": %m", argv[1]);
	}
	if (tasks > USHRT_MAX) {
		die_ret("Too high number of tasks \"%s\": %m.", argv[1]);
	}

	/*
	 * Initialize the semaphore.
	 * We won't auto-cleanup it because it's IPC_PRIVATE.
	 */

	int sem = semget(IPC_PRIVATE, 1, 0600);
	if (sem < 0) {
		die_ret("Failed to semget() the control semaphore: %m");
	}

	r = semctl(sem, 0, SETVAL, sem_arg_val((int)tasks));
	if (r < 0) {
		die_ret("Failed to call semctl(SETVAL) to initialize the control semaphore %d: %m", sem);
	}

	/*
	 * Ignore the SIGCHLD -- let the kernel reap children for us.
	 */

	signal(SIGCHLD, SIG_IGN);

	/*
	 * Now read the input line-by-line and fork a child on each line, which will P the semaphore
	 * with SEM_UNDO and immediately exec().
	 */

	FILE *in_file = stdin;

	for (;;) {
		_cleanup_free_ char *line = NULL;
		r = fscanf(in_file, " %m[^\n]", &line);
		if (r < 1) {
			if (feof(in_file)) {
				break;
			} else if (ferror(in_file)) {
				die_ret("Failed to read line from input file: %m");
			} else {
				die_ret("Failed to parse input file.");
			}
		}

		/*
		 * Tokenize the input string.
		 */

		_cleanup_free_ char **tokens = malloc(sizeof(char*));
		size_t tokens_alloc = 1, tokens_nr = 0;
		char *strtok_line = line, *strtok_saveptr = NULL;

		assert(tokens);

		for (;;) {
			char *next_token = strtok_r(strtok_line, " ", &strtok_saveptr);
			strtok_line = NULL;

			if (tokens_nr >= tokens_alloc) {
				tokens_alloc *= 2;
				tokens = realloc(tokens, sizeof(char*) * tokens_alloc);
				assert(tokens);
			}

			tokens[tokens_nr++] = next_token;
			if (next_token == NULL) {
				break;
			}
		}

		if (tokens_nr == 1) {
			log("Invalid input line \"%s\".", line);
		}

		/*
		 * Fork off the child, P the control semaphore and execvp() the desired executable.
		 */

		int child_pid = fork();
		if (child_pid == 0) {
			r = semop_one(sem, 0, -1, SEM_UNDO);
			if (r < 0) {
				die_ret("Failed to P the control semaphore %d: %m", sem);
			}

			r = execvp(tokens[0], tokens);
			die_ret("Failed to execvp() the file \"%s\": %m", tokens[0]);
		} else if (child_pid < 0) {
			die_ret("Failed to fork() off a child: %m");
		}
	}

	/*
	 * Wait for direct children.
	 * As we've earlier ignored SIGCHLD, this call to wait() will wait for everybody and _then_ fail with ECHILD.
	 */

	r = wait(NULL);
	if (r >= 0 || errno != ECHILD) {
		log("Unexpected wait() result %d: %m", r);
	}

	return 0;
}
