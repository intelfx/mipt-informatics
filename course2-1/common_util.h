#pragma once

#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <string.h>

/*
 * Preprocessor
 */

#define container_of(ptr, type, member)                         \
	(type *)((char *)(ptr) - offsetof(type, member))

#define memzero(x) memset(&(x), 0, sizeof(x))

/*
 * Logging
 */

#define log(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define die_ret(fmt, ...) do { log(fmt, ## __VA_ARGS__); return -1; } while (0)
#define die_out(fmt, ...) do { log(fmt, ## __VA_ARGS__); goto out; } while (0)
#define chk(expr, fmt, ...) do { if (!(expr)) die(fmt, ## __VA_ARGS__); } while (0)

/*
 * RAII-style "destructors" for C
 */

#define _cleanup_(x) \
        __attribute__((cleanup(x)))

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)                 \
        static inline void func##p(type *p) {                   \
                if (*p)                                         \
                        func(*p);                               \
        }

#define DEFINE_TRIVIAL_CLEANUP_FUNC_UNSAFE(type, func)          \
        static inline void func##p(void *p) {                   \
                if (*(type **)p)                                \
                        func(*(type **)p);                      \
        }

DEFINE_TRIVIAL_CLEANUP_FUNC_UNSAFE(void*, free)
#define _cleanup_free_ \
	_cleanup_(freep)

static inline void safe_fclose(FILE* file)
{
        if (file != NULL)
                fclose(file);
}
DEFINE_TRIVIAL_CLEANUP_FUNC(FILE*, safe_fclose)
#define _cleanup_fclose_ \
	_cleanup_(safe_fclosep)

static inline void safe_close(int fd)
{
        if (fd >= 0)
                close(fd);

}
DEFINE_TRIVIAL_CLEANUP_FUNC(int, safe_close)
#define _cleanup_close_ \
	_cleanup_(safe_closep)

/*
 * Misc
 */

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
	long val = strtol(arg, &endptr, 10);

	if (*arg == '\0' || *endptr != '\0') {
		die("Argument is not a valid decimal number: '%s'", arg);
	}

	if (errno) {
		die("Argument parse error: '%s': %m", arg);
	}

	return val;
}

unsigned long strtoul_or_die(const char *arg)
{
	char *endptr = NULL;
	errno = 0;
	unsigned long val = strtoul(arg, &endptr, 10);

	if (*arg == '\0' || *endptr != '\0') {
		die("Argument is not a valid non-negative decimal number: '%s'", arg);
	}

	if (errno) {
		die("Argument parse error: '%s': %m", arg);
	}

	return val;
}

void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);

	if (!result) {
		die("Failed to malloc() %zu bytes of memory: %m", bytes);
	}

	return result;
}
