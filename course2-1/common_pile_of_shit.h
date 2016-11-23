#pragma once

#include "common_util.h"

#define _GNU_SOURCE 1
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <sys/poll.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <locale.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

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
