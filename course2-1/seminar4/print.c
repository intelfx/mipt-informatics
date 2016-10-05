#include "common.h"

int main(int argc, char **argv)
{
	if (argc != 1) {
		die("This program expects no arguments.");
	}

	int r;
	ssize_t rss;
	const char *fifo_path = NULL, *fifo_ctl_path = "fifo-control";
	int pipe_rd_fd = -1, pipe_ctl_rd_fd = -1, out_fd = STDOUT_FILENO;
	pid_t server_pid = -1;

	r = mkfifo(fifo_ctl_path, 0600);
	if (r < 0 && errno != EEXIST) {
		log("Failed to mkfifo() control FIFO \"%s\": %m", fifo_ctl_path);
		r = 1;
		goto cleanup;
	}

	/* blocks until server appear, servers open() and block on writing, we unblock and continue */
	pipe_ctl_rd_fd = open(fifo_ctl_path, O_RDONLY);
	if (pipe_ctl_rd_fd < 0) {
		log("Failed to open() read end on control FIFO \"%s\": %m", fifo_ctl_path);
		r = 1;
		goto cleanup;
	}

	/* atomic read() */
	rss = read(pipe_ctl_rd_fd, &server_pid, sizeof(server_pid));
	if (rss != sizeof(server_pid)) {
		log("Failed to read() %zu bytes from control FIFO \"%s\", read %zd bytes: %m",
		    sizeof(server_pid), fifo_ctl_path, rss);
		r = 1;
		goto cleanup;
	}

	/* here server waits for us with a timeout... */
	pipe_rd_fd = open_rdonly_no_wait((fifo_path = snprintf_static("fifo-%d", server_pid)), 0);
	if (pipe_rd_fd < 0) {
		log("Failed to open() read end on \"%s\": %m", fifo_path);
		r = 1;
		goto cleanup;
	}

	/* read while we can */
	r = cat_fd(pipe_rd_fd, out_fd, pipe_buffer_size(pipe_rd_fd), 0);
	switch (r) {
	case RESULT_READ_NIL:
	case RESULT_OK:
		r = 0;
		break;

	default:
		r = 1;
		break;
	}

cleanup:
	close(pipe_rd_fd);
	close(pipe_ctl_rd_fd);
	return r;
}
