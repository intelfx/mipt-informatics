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

int open_wronly_no_wait(const char *path, int flags)
{
	/*
	 * We can't simply open a pipe in O_WRONLY mode without waiting for readers.
	 * Here's how we do it.
	 */

	int r;
	int rd_fd = -1, wr_fd = -1;

	rd_fd = open(path, O_RDONLY|O_NONBLOCK);
	if (rd_fd < 0) {
		log("Failed to transiently open() \"%s\" in read-only non-blocking mode: %m", path);
		r = -1;
		goto cleanup;
	}

	wr_fd = open(path, O_WRONLY|flags|O_NONBLOCK);
	if (wr_fd < 0) {
		log("Failed to transiently open() \"%s\" in write-only non-blocking mode: %m", path);
		r = -1;
		goto cleanup;
	}

	r = fcntl(wr_fd, F_SETFL, flags);
	if (r < 0) {
		log("Failed to fcntl(F_SETFL) to clear O_NONBLOCK on write-only fd %d for file \"%s\": %m",
		    wr_fd, path);
		goto cleanup;
	}

	close(rd_fd);
	return wr_fd;

cleanup:
	close(rd_fd);
	close(wr_fd);
	return r;
}

int open_rdonly_no_wait(const char *path, int flags)
{
	/*
	 * We can't simply open a pipe in blocking O_RDONLY mode without waiting for writers.
	 * Here's how we do it.
	 */

	int r;
	int rd_fd = -1;

	rd_fd = open(path, O_RDONLY|flags|O_NONBLOCK);
	if (rd_fd < 0) {
		log("Failed to open() \"%s\" in read-only non-blocking mode: %m", path);
		r = -1;
		goto cleanup;
	}

	r = fcntl(rd_fd, F_SETFL, flags);
	if (r < 0) {
		log("Failed to fcntl(F_SETFL) to clear O_NONBLOCK on read-only fd %d for file \"%s\": %m",
		    rd_fd, path);
		goto cleanup;
	}

	return rd_fd;

cleanup:
	close(rd_fd);
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

int cat_fd_iter(int fd_in, int fd_out, char *buf, size_t buffer_size, int retry)
{
	ssize_t read_bytes, written_bytes;

	read_bytes = read(fd_in, buf, buffer_size);

	if (read_bytes < 0) {
		log("Failed to read() %zu bytes: %m", buffer_size);
		return RESULT_READ_ERR;
	} else if (read_bytes == 0) {
		return RESULT_READ_NIL;
	}

	for (;;) {
		written_bytes = write(fd_out, buf, read_bytes);
		if (retry > 0 && written_bytes < 0 && errno == EPIPE) {
			log("Failed to write() %zu bytes, retrying: %m", buffer_size);
			sleep(1);
			--retry;
			continue;
		}
		break;
	}

	if (written_bytes < 0) {
		log("Failed to write() %zd bytes: %m", read_bytes);
		return RESULT_WRITE_ERR;
	} else if (written_bytes != read_bytes) {
		log("Short write(): %zd bytes out of %zd: %m", written_bytes, read_bytes);
		return RESULT_WRITE_ERR;
	}

	return RESULT_OK;
}

int cat_fd(int fd_in, int fd_out, size_t buffer_size, int retry)
{
	int r;
	char *buf = malloc(buffer_size);

	if (!buf) {
		log("Failed to malloc() buffer of %zu bytes: %m", buffer_size);
		return 1;
	}


	/*
	 * First iteration is separated to detect early EOFs.
	 */
	
	r = cat_fd_iter(fd_in, fd_out, buf, buffer_size, retry);
	if (r != RESULT_OK) {
		/*
		 * Either an error or an EOF-before-all.
		 */
		goto cleanup;
	}

	for (;;) {
		r = cat_fd_iter(fd_in, fd_out, buf, buffer_size, 0);
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
