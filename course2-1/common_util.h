#pragma once

#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>

#define log(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define die_ret(fmt, ...) do { log(fmt, ## __VA_ARGS__); return 1; } while (0)
#define chk(expr, fmt, ...) do { if (!(expr)) die(fmt, ## __VA_ARGS__); } while (0)

char *snprintf_static(const char *fmt, ...)
{
	static char buf[1024];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	return buf;
}

int pipe_buffer_size(int fd)
{
	int r;

#ifdef F_GETPIPE_SZ
		r = fcntl(fd, F_GETPIPE_SZ);
		if (r < 0)
#endif // F_GETPIPE_SZ
			r = PIPE_BUF;

		return r;
}

long strtol_or_die(const char *arg)
{
	char *endptr = NULL;
	errno = 0;
	long val = strtol (arg, &endptr, 10);

	if (*arg == '\0' || *endptr != '\0') {
		die("Argument is not a valid decimal number: '%s'", arg);
	}

	if (errno) {
		die("Argument parse error: '%s': %m", arg);
	}

	return val;
}
