#include "common.h"

static const int FIFO_NR = 1000;

int main(int argc, char **argv)
{
	if (argc != 1) {
		die("This program expects no arguments.");
	}

	int r;
	const char *fifo_path = NULL;
	int pipe_rd_fd = -1, out_fd = STDOUT_FILENO;

	char *unique_name_buf = alloca(1024);
	snprintf(unique_name_buf, 1024, "fifo-owned-%d", getpid());

	for (int i = 1; i <= FIFO_NR; ++i) {
		r = rename((fifo_path = snprintf_static("fifo-%d", i)), unique_name_buf);
		if (r < 0) {
			if (errno == ENOENT)
				continue;
			log("Failed to rename() fifo \"%s\" -> \"%s\": %m", fifo_path, unique_name_buf);
			r = 1;
			goto cleanup;
		}

		/*
		 * We detect pipes-without-writer at first read, so we need to be able to
		 * successfully open such pipes (and not block on open()). Hence open with O_NONBLOCK
		 * and clear it right afterwards.
		 */

		pipe_rd_fd = open(unique_name_buf, O_RDONLY|O_NONBLOCK);
		if (pipe_rd_fd < 0) {
			if (errno == ENOENT)
				continue;
			log("Failed to open() read end on \"%s\": %m", fifo_path);
			r = 1;
			goto cleanup;
		}

		r = unlink(unique_name_buf);
		if (r < 0) {
			log("Failed to unlink() \"%s\": %m", fifo_path);
			r = 1;
			goto cleanup;
		}

		r = fcntl(pipe_rd_fd, F_GETFL);
		if (r < 0) {
			log("Failed to fcntl(F_GETFL): %m");
			r = 1;
			goto cleanup;
		}

		r = fcntl(pipe_rd_fd, F_SETFL, r & ~O_NONBLOCK);
		if (r < 0) {
			log("Failed to fcntl(F_SETFL) to clear O_NONBLOCK: %m");
			r = 1;
			goto cleanup;
		}

		/*
		 * Do the copy inside the pipe choose loop as we may decide to continue search
		 * if we happen to be reading a pipe without writer.
		 * 
		 * In our model, readers do not wait for writers, so a pipe without writers is
		 * necessarily a stale one.
		 */

		r = cat_fd(pipe_rd_fd, out_fd, pipe_buffer_size(pipe_rd_fd));
		switch (r) {
		/*
		 * Got early EOF, hence the pipe has no readers.
		 * Continue to the next one.
		 */
		case RESULT_READ_NIL:
			close(pipe_rd_fd);
			continue;

		case RESULT_OK:
			r = 0;
			goto cleanup;

		default:
			r = 1;
			goto cleanup;
		}
	}

	log("No fifo found to read from.");
	r = 1;

cleanup:
	close(pipe_rd_fd);
	return r;
}
