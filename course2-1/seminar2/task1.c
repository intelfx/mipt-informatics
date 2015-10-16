#include <unistd.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "This program expects at least one argument.\n");
		return 1;
	}

	int arg_cnt = argc - 1;
	char **exec_argv = alloca(sizeof(char*) * (arg_cnt + 1));
	memcpy(exec_argv, argv + 1, sizeof(char*) * arg_cnt);
	exec_argv[arg_cnt] = NULL;

	execvp(exec_argv[0], exec_argv);
	fprintf(stderr, "Failed to execvp(): %m\n");
	return 1;
}
