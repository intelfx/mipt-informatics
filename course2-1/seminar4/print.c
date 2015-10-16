#include "common.h"

static const int FIFO_NR = 1000;

int main(int argc, char **argv)
{
	if (argc != 1) {
		fprintf(stderr, "This program expects no arguments.\n");
		return 1;
	}

	int r;
	const char *fifo_path = NULL;
	int pipe_rd_fd = -1, out_fd = STDOUT_FILENO;

	char *unique_name_buf = alloca(1024);
	snprintf(unique_name_buf, 1024, "fifo-owned-%d", getpid());

	for (int i = 1; i <= FIFO_NR; ++i) {
		if (i < 0) {
			fprintf(stderr, "No fifo found to read from.\n");
			r = 1;
			goto cleanup;
		}

		r = rename((fifo_path = snprintf_static("fifo-%d", i)), unique_name_buf);
		if (r < 0) {
			if (errno == ENOENT)
				continue;
			fprintf(stderr, "Failed to rename() fifo \"%s\": %m", fifo_path);
			r = 1;
			goto cleanup;
		}

		pipe_rd_fd = open(unique_name_buf, O_RDONLY|O_NONBLOCK);
		if (pipe_rd_fd < 0) {
			if (errno == ENOENT)
				continue;
			fprintf(stderr, "Failed to open() \"%s\": %m\n", fifo_path);
			r = 1;
			goto cleanup;
		}

		r = unlink(unique_name_buf);
		if (r < 0) {
			fprintf(stderr, "Failed to unlink() \"%s\": %m\n", fifo_path);
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

		r = cat_fd(pipe_rd_fd, out_fd, pipe_buffer_size(pipe_rd_fd), 1);
		switch (r) {
		/*
		 * Got EOF while the fifo read fd was still non-blocking, hence the pipe has no readers.
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


cleanup:
	close(pipe_rd_fd);
	return r;
}
