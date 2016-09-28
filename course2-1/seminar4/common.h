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

#define log(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define die_ret(fmt, ...) do { log(fmt, ## __VA_ARGS__); return 1; } while (0)

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

enum cat_fd_result
{
	RESULT_OK,
	RESULT_READ_NIL,  /* EOF */
	RESULT_READ_ERR,
	RESULT_WRITE_ERR, /* or a short write */
	RESULT_MISC_ERR
};

int cat_fd_iter(int fd_in, int fd_out, char *buf, size_t buffer_size)
{
	ssize_t read_bytes, written_bytes;

	read_bytes = read(fd_in, buf, buffer_size);

	if (read_bytes < 0) {
		fprintf(stderr, "Failed to read() %zd bytes: %m\n", buffer_size);
		return RESULT_READ_ERR;
	} else if (read_bytes == 0) {
		return RESULT_READ_NIL;
	}

	written_bytes = write(fd_out, buf, read_bytes);

	if (written_bytes < 0) {
		fprintf(stderr, "Failed to write() %zd bytes: %m\n", read_bytes);
		return RESULT_WRITE_ERR;
	} else if (written_bytes != read_bytes) {
		fprintf(stderr, "Short write(): %zd bytes out of %zd: %m\n", written_bytes, read_bytes);
		return RESULT_WRITE_ERR;
	}

	return RESULT_OK;
}

int cat_fd(int fd_in, int fd_out, size_t buffer_size)
{
	int r;
	char *buf = malloc(buffer_size);

	if (!buf) {
		fprintf(stderr, "Failed to malloc() buffer of %zu bytes: %m\n", buffer_size);
		return 1;
	}


	/*
	 * First iteration is separated to detect early EOFs.
	 */
	
	r = cat_fd_iter(fd_in, fd_out, buf, buffer_size);
	if (r != RESULT_OK) {
		/*
		 * Either an error or an EOF-before-all.
		 */
		goto cleanup;
	}

	for (;;) {
		r = cat_fd_iter(fd_in, fd_out, buf, buffer_size);
		switch (r) {
		/*
		 * EOF after we've read something is OK.
		 */
		case RESULT_READ_NIL:
			r = RESULT_OK;
			goto cleanup;

		/*
		 * OK means continue to read.
		 */
		case RESULT_OK:
			break;

		/*
		 * Otherwise, it's an error.
		 */
		default:
			goto cleanup;
		}
	}

cleanup:
	free(buf);
	return r;
}
