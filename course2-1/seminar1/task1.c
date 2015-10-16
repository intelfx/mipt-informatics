#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main (int argc, char **argv)
{
	if (argc != 2) {
		fprintf (stderr, "This program expects one argument.\n");
		return 1;
	}

	char *endptr = NULL;
	errno = 0;
	long arg = strtol (argv[1], &endptr, 10);

	if (*argv[1] == '\0' || *endptr != '\0') {
		fprintf (stderr, "Argument is not a valid decimal number: '%s'\n", argv[1]);
		return 1;
	}

	if (errno) {
		fprintf (stderr, "Argument parse error: '%s': %m\n", argv[1]);
		return 1;
	}

	for (long i = 1; i <= arg; ++i) {
		printf ("%ld\n", i);
	}

	return 0;
}
