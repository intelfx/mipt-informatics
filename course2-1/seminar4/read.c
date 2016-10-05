#include "common.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		log("This program expects one argument.");
		return 1;
	}

	int r;
	ssize_t rss;
	const char *in_path = argv[1], *fifo_path = NULL, *fifo_ctl_path = "fifo-control";
	int in_fd = -1, pipe_wr_fd = -1, pipe_ctl_wr_fd = -1;
	pid_t my_pid = getpid();

	in_fd = open(in_path, O_RDONLY);
	if (in_fd < 0) {
		log("Failed to open() input: %m");
		r = 1;
		goto cleanup;
	}

	r = mkfifo(fifo_ctl_path, 0600);
	if (r < 0 && errno != EEXIST) {
		log("Failed to mkfifo() control FIFO \"%s\": %m", fifo_ctl_path);
		r = 1;
		goto cleanup;
	}

	r = mkfifo((fifo_path = snprintf_static("fifo-%d", my_pid)), 0600);
	if (r < 0 && errno != EEXIST) {
		log("Failed to mkfifo() unique FIFO \"%s\": %m", fifo_path);
		r = 1;
		goto cleanup;
	}

	/* blocks until clients appear, clients open() and block on reading, we unblock and continue */
	pipe_ctl_wr_fd = open(fifo_ctl_path, O_WRONLY);
	if (pipe_ctl_wr_fd < 0) {
		log("Failed to open() write end on control FIFO \"%s\": %m", fifo_ctl_path);
		r = 1;
		goto cleanup;
	}

	/* does not block until a client open()s a FIFO for reading, we'd rather block on write() */
	pipe_wr_fd = open_wronly_no_wait(fifo_path, 0);
	if (pipe_wr_fd < 0) {
		log("Failed to open() write end on \"%s\": %m", fifo_path);
		r = 1;
		goto cleanup;
	}

	/* atomic write() */
	rss = write(pipe_ctl_wr_fd, &my_pid, sizeof(my_pid));
	if (rss != sizeof(my_pid)) {
		log("Failed to write() %zu bytes into control FIFO \"%s\", wrote %zd bytes: %m",
		    sizeof(my_pid), fifo_ctl_path, rss);
		r = 1;
		goto cleanup;
	}

	/* block SIGPIPE so write() below will get EPIPE if there is no reader */
	signal(SIGPIPE, SIG_IGN);

	/* write() until client dies, in which case we get EPIPE */
	r = cat_fd(in_fd, pipe_wr_fd, pipe_buffer_size(pipe_wr_fd), 10);
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
	unlink(fifo_path);
	close(pipe_ctl_wr_fd);
	close(pipe_wr_fd);
	close(in_fd);
	return r;
}
