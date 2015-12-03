#include "common.h"

static void cleanup(void)
{
	unlink(get_pipe_server_path());
	unlink(get_pipe_server_guard_path());
}

static void cleanup_signal(int sig)
{
	cleanup();

	/* re-raise the signal */
	signal(sig, SIG_DFL);
	raise(sig);
}

static int process_request(struct Request *request)
{
	int r;
	_cleanup_close_ int pipe_slave_wr_fd = -1, file_fd = -1;
	const char *pipe_slave_path;

	assert(request);

	file_fd = open(request->path, O_RDONLY);
	if (file_fd < 0) {
		log("Failed to open() the requested file: %m");
		return -1;
	}

	/*
	 * The client's FIFO should be open at this point. Open it in nonblocking mode and check for ENXIO
	 * to protect against dead clients.
	 */

	pipe_slave_path = get_pipe_client_path_by_pid(request->pid);
	pipe_slave_wr_fd = open(pipe_slave_path, O_WRONLY|O_NONBLOCK);
	if (pipe_slave_wr_fd < 0) {
		if (errno == ENXIO) {
			log("[Dead client %d] Failed to open() the writing end of slave FIFO \"%s\": %m", request->pid, pipe_slave_path);
			return 0;
		} else {
			log("Failed to open() the writing end of slave FIFO \"%s\": %m", pipe_slave_path);
			return -1;
		}
	}

	/*
	 * However, we want our writes to be blocking (if client drops out, we'll be told EPIPE).
	 */

	r = fd_make_blocking(pipe_slave_wr_fd);
	if (r < 0) {
		log("Failed to set blocking mode on the writing end of slave FIFO: %m");
		return -1;
	}

	r = fd_cat(file_fd, pipe_slave_wr_fd, 4096); /* one "common" page size worth of a buffer. why not? */
	if (r != RESULT_OK) {
		if (errno == EPIPE) {
			log("[Dead client %d] Failed to copy data to the writing end of slave FIFO: %m", request->pid);
			return 0;
		} else {
			log("Failed to copy data to the writing end of slave FIFO: %m");
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 1) {
		die("This program expects no arguments.");
	}

	int r;
	const char *pipe_master_path = get_pipe_server_path(),
	           *pipe_guard_path = get_pipe_server_guard_path();
	_cleanup_close_ int pipe_guard_wr_fd = -1;

	/*
	 * Configure cleanup.
	 */

	atexit(&cleanup);
	signal(SIGTERM, &cleanup_signal);
	signal(SIGHUP, &cleanup_signal);
	signal(SIGINT, &cleanup_signal);
	signal(SIGPIPE, SIG_IGN);

	/*
	 * Create and open the guard FIFO.
	 * We cannot open the FIFO writing end without readers even with O_NONBLOCK.
	 */
	r = unlink_and_mkfifo(pipe_guard_path, 0666);
	if (r < 0) {
		die("Failed to make guard FIFO \"%s\": %m", pipe_guard_path);
	}

	pipe_guard_wr_fd = open_wronly_nonblock(pipe_guard_path, 0);
	if (pipe_guard_wr_fd < 0) {
		die("Failed to open guard FIFO \"%s\" in write-only non-blocking mode: %m", pipe_guard_path);
	}

	/*
	 * Create the master FIFO. We open it repeatedly in the loop below.
	 */

	r = unlink_and_mkfifo(pipe_master_path, 0666);
	if (r < 0) {
		die("Failed to make master FIFO \"%s\": %m", pipe_master_path);
	}


	/*
	 * Process requests.
	 */

	for (;;) {
		_cleanup_close_ int pipe_master_rd_fd = -1;

		/*
		 * Open the master FIFO.
		 * The reason to do it in the loop is that once a FIFO is opened, we cannot make read() block
		 * on absence of writers, waiting for new clients (and O_RDWR on a FIFO is undefined in POSIX).
		 * Hence on receipt of an EOF on the master FIFO we close it and open it again.
		 */

		pipe_master_rd_fd = open(pipe_master_path, O_RDONLY);
		if (pipe_master_rd_fd < 0) {
			die("Failed to open() the reading end of master FIFO \"%s\": %m", pipe_master_path);
		}

		/*
		 * Now wait for data on the master FIFO.
		 */

		_cleanup_free_ struct Request *request = NULL;

		for (;;) {
			r = read_request(pipe_master_rd_fd, (void**)&request);
			if (r == 0) {
				break;
			} else if (r < 0) {
				die("Failed to read next request from the master FIFO: %m");
			}

			assert(request);
			r = process_request(request);
			if (r < 0) {
				die("Failed to reply to a request by pid %d to read file \"%s\": %m", request->pid, request->path);
			}
		}
	}

	return 0;
}
