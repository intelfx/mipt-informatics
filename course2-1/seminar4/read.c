#include "common.h"

static const int FIFO_NR = 1000;

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "This program expects one argument.\n");
		return 1;
	}

	int r;
	const char *in_path = argv[1], *fifo_path = NULL;
	int in_fd = -1, pipe_wr_fd = -1;

	in_fd = open(in_path, O_RDONLY);
	if (in_fd < 0) {
		fprintf(stderr, "Failed to open() input: %m\n");
		r = 1;
		goto cleanup;
	}

	for (int i = 1; i <= FIFO_NR; ++i) {
		if (i < 0) {
			fprintf(stderr, "Too many pipes already exist, cannot make a free index.\n");
			r = 1;
			goto cleanup;
		}

		r = mkfifo((fifo_path = snprintf_static("fifo-%d", i)), 0600);
		if (r < 0) {
			if (errno == EEXIST)
				continue;
			fprintf(stderr, "Failed to mkfifo() \"%s\": %m\n", fifo_path);
			r = 1;
			goto cleanup;
		}

		break;
	}

	pipe_wr_fd = open(fifo_path, O_WRONLY);
	if (pipe_wr_fd < 0) {
		fprintf(stderr, "Failed to open() write end on \"%s\": %m\n", fifo_path);
		r = 1;
		goto cleanup;
	}

	r = cat_fd(in_fd, pipe_wr_fd, pipe_buffer_size(pipe_wr_fd));
	switch (r) {
	case RESULT_OK:
		r = 0;
		break;

	default:
		r = 1;
		break;
	}

cleanup:
	unlink(fifo_path);
	close(pipe_wr_fd);
	close(in_fd);
	return r;
}
