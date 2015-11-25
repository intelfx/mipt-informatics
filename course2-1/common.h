#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <locale.h>

/*
 * Logging
 */

#define log(fmt, ...) do { int _errno = errno; fprintf (stderr, fmt "\n", ## __VA_ARGS__); errno = _errno; } while (0)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)
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
 * Various operations
 */

#define elementsof(array) (sizeof(array) / sizeof(array[0]))
#define memzero(array) memset(&array, 0, sizeof(array))

static inline bool streq(const char *_1, const char *_2)
{
	assert(_1);
	assert(_2);

	return !strcmp(_1, _2);
}

static inline bool strempty(const char *arg)
{
	assert(arg);

	return arg[0] == '\0';
}

static inline char *snprintf_static(const char *fmt, ...)
{
	static char buf[1024];
	va_list ap;

	assert(fmt);

	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	return buf;
}

static inline int unlink_and_mkfifo(const char *path, int mode)
{
	int r;

	assert(path);

	r = unlink(path);
	if (r < 0 && errno != ENOENT) {
		log("Failed to unlink() \"%s\": %m", path);
		goto out;
	}

	r = mkfifo(path, mode);
	if (r < 0) {
		die("Failed to mkfifo() FIFO \"%s\": %m", path);
		goto out;
	}

out:
	return r;
}

static inline void astrftime(char **result, const char *format, time_t time)
{
	struct tm tm;
	_cleanup_free_ char *buf = NULL;
	size_t buf_size, r;

	assert(result);
	assert(format);

	buf_size = 32;
	buf = malloc(buf_size);
	assert(buf);

	localtime_r(&time, &tm);
	for (;;) {
		r = strftime(buf, buf_size, format, &tm);
		if (r != 0) {
			break;
		}
		buf_size *= 2;
		buf = realloc(buf, buf_size);
		assert(buf);
	}

	*result = buf;
	buf = NULL;
}

/*
 * Message send/receive over FIFO
 */

struct MessageHeader
{
	size_t length; /* including the header */
} __attribute__((packed));

static inline int read_request(int fd, void **pbuffer)
{
	int r;
	struct MessageHeader *header;
	ssize_t payload_length;

	assert(pbuffer);
	header = *pbuffer;

	if (header == NULL) {
		header = malloc(sizeof(*header));
		assert(header);
	}

	r = read(fd, header, sizeof(*header));
	if (r == 0) {
		goto out;
	}
	if (r != sizeof(*header)) {
		r = -1;
		log("Failed to read() message header (%zu bytes) from fd %d: %m", sizeof(*header), fd);
		goto out;
	}

	payload_length = header->length - sizeof(*header);
	if (payload_length > 0) {
		header = realloc(header, header->length);
		assert(header);

		r = read(fd, header + 1, payload_length);
		if (r != payload_length) {
			r = -1;
			log("Failed to read() message payload (%zu bytes) from fd %d: %m", payload_length, fd);
			goto out;
		}
	}

out:
	*pbuffer = header;
	return r;
}

static inline int make_request(void **pbuffer, size_t size)
{
	struct MessageHeader *header;

	assert(!*pbuffer);

	header = malloc(size);
	assert(header);

	header->length = size;

	*pbuffer = header;
	return 0;
}

static inline int send_request(int fd, struct MessageHeader *buffer)
{
	int r;

	assert(buffer);

	if (buffer->length > PIPE_BUF) {
		log("Will not send a request of %zu bytes (which is more than PIPE_BUF) because it is not atomic.", buffer->length);
		errno = ENOSPC;
		return -1;
	}

	r = write(fd, buffer, buffer->length);
	if (r < 0 || (unsigned)r != buffer->length) {
		log("Failed to write() message header (%zu bytes) to fd %d: %m", buffer->length, fd);
		return -1;
	}

	return 0;
}

/*
 * Various fd operations.
 */

static inline int open_wronly_nonblock(const char *path, int flags)
{
	_cleanup_close_ int rd_fd = -1;
	int wr_fd;

	assert(path);

	rd_fd = open(path, O_RDONLY|O_NONBLOCK);
	if (rd_fd < 0) {
		log("Failed to open() reading end of \"%s\" in non-blocking mode: %m", path);
		return -1;
	}

	wr_fd = open(path, O_WRONLY|O_NONBLOCK|flags);
	if (wr_fd < 0) {
		log("Failed to open() writing end of \"%s\" in non-blocking mode with reading end opened: %m", path);
		return -1;
	}

	return wr_fd;
}

static inline int fd_make_blocking(int fd)
{
	int r;

	r = fcntl(fd, F_GETFL);
	if (r < 0) {
		log("Failed to fcntl(F_GETFL) fd %d: %m", fd);
		return -1;
	}

	if (r & O_NONBLOCK) {
		r = fcntl(fd, F_SETFL, r & ~O_NONBLOCK);
		if (r < 0) {
			log("Failed to fcntl(F_SETFL) fd %d to make blocking: %m", fd);
			return -1;
		}
	}

	return 0;
}

enum fd_cat_result
{
	RESULT_OK,
	RESULT_READ_NIL,  /* EOF */
	RESULT_READ_ERR,
	RESULT_WRITE_ERR, /* or a short write */
	RESULT_MISC_ERR
};

static inline int fd_cat_iter(int fd_in, int fd_out, char *buf, size_t buffer_size)
{
	ssize_t read_bytes, written_bytes;

	assert(buf);

	read_bytes = read(fd_in, buf, buffer_size);

	if (read_bytes < 0) {
		log("Failed to read() %zd bytes: %m", buffer_size);
		return RESULT_READ_ERR;
	} else if (read_bytes == 0) {
		return RESULT_READ_NIL;
	}

	written_bytes = write(fd_out, buf, read_bytes);

	if (written_bytes < 0) {
		log("Failed to write() %zd bytes: %m", read_bytes);
		return RESULT_WRITE_ERR;
	} else if (written_bytes != read_bytes) {
		log("Short write(): %zd bytes out of %zd: %m", written_bytes, read_bytes);
		return RESULT_WRITE_ERR;
	}

	return RESULT_OK;
}

static inline int fd_cat(int fd_in, int fd_out, size_t buffer_size)
{
	int r;
	char *buf = malloc(buffer_size);

	if (!buf) {
		log("Failed to malloc() buffer of %zu bytes: %m", buffer_size);
		return RESULT_MISC_ERR;
	}


	/*
	 * First iteration is separated to detect early EOFs.
	 */

	r = fd_cat_iter(fd_in, fd_out, buf, buffer_size);
	if (r != RESULT_OK) {
		/*
		 * Either an error or an EOF-before-all.
		 */
		goto cleanup;
	}

	for (;;) {
		r = fd_cat_iter(fd_in, fd_out, buf, buffer_size);
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

/*
 * Domain-specific definitions (shared between server and client)
 */

struct Request
{
	struct MessageHeader hdr;
	int pid;
	char path[1];
} __attribute__((packed));

static inline const char *get_pipe_server_path(void)
{
	return "fifo.master";
}

static inline const char *get_pipe_server_guard_path(void)
{
	return "fifo.master-guard";
}

static inline const char *get_pipe_client_path_by_pid(int pid)
{
	return snprintf_static("fifo.slave-%d", pid);
}
