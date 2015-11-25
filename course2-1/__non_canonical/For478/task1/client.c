#include "../../../common.h"

static void cleanup(void)
{
	unlink(get_pipe_client_path_by_pid(getpid()));
}

static void cleanup_signal(int sig)
{
	cleanup();

	/* re-raise the signal */
	signal(sig, SIG_DFL);
	raise(sig);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		die("This program expects one argument.");
	}

	int r;
	const char *file = argv[1];
	const char *pipe_master_path = get_pipe_server_path(),
	           *pipe_guard_path = get_pipe_server_guard_path(),
	           *pipe_slave_path = get_pipe_client_path_by_pid(getpid());
	_cleanup_close_ int pipe_guard_rd_fd = -1,
	                    pipe_master_wr_fd = -1,
	                    pipe_slave_rd_fd = -1;
	int out_fd = STDOUT_FILENO;

	/*
	 * Configure cleanup.
	 */

	atexit(&cleanup);
	signal(SIGTERM, &cleanup_signal);
	signal(SIGHUP, &cleanup_signal);
	signal(SIGINT, &cleanup_signal);
	signal(SIGPIPE, SIG_IGN);

	/*
	 * Open the server FIFOs.
	 *
	 * We use two server-created FIFOs: one main, of which clients open the writing end and atomically
	 * write their requests there, while the server dequeues and processes them, and another one "guard",
	 * which is opened by the server for writing and by clients for reading.
	 *
	 * No one actually writes to or reads from the guard pipe, but once server dies, any client wishing
	 * to read from that pipe will get EOF instead of blocking or getting EAGAIN, which can be checked
	 * in a separate thread or by using poll().
	 *
	 * We open both FIFOs in non-blocking mode to avoid blocking on dead servers; we'd rather detect this
	 * condition explicitly and tell the user. The master FIFO is then made blocking as we write to it,
	 * and we don't bother doing the same on guard FIFO because poll() does not care.
	 */

	pipe_master_wr_fd = open(pipe_master_path, O_WRONLY|O_NONBLOCK);
	if (pipe_master_wr_fd < 0) {
		if (errno == ENXIO) {
			die("[Dead server] Failed to open() the writing end of master FIFO \"%s\" in non-blocking mode: %m", pipe_master_path);
		} else if (errno == ENOENT) {
			die("[Ordering error] Failed to open() the writing end of master FIFO \"%s\" in non-blocking mode: %m", pipe_master_path);
		} else {
			die("Failed to open() the writing end of master FIFO \"%s\" in non-blocking mode: %m", pipe_master_path);
		}
	}

	pipe_guard_rd_fd = open(pipe_guard_path, O_RDONLY|O_NONBLOCK);
	if (pipe_guard_rd_fd < 0) {
		die("Failed to open() the reading end of guard FIFO \"%s\" in non-blocking mode: %m", pipe_guard_path);
	}

	r = fd_make_blocking(pipe_master_wr_fd);
	if (r < 0) {
		die("Failed to enable blocking mode on the writing end of master FIFO: %m");
	}

	/*
	 * Create our FIFO and open it in a non-blocking mode to tell server we are alive. We will wait for
	 * actual data later by using poll().
	 *
	 * If we happen to die, the server will get ENXIO or EPIPE (sufficient), and due to this early open()
	 * it will never get ENXIO or EPIPE otherwise (requisite).
	 */

	r = unlink_and_mkfifo(pipe_slave_path, 0666);
	if (r < 0) {
		die("Failed to make slave FIFO \"%s\": %m", pipe_slave_path);
	}

	pipe_slave_rd_fd = open(pipe_slave_path, O_RDONLY|O_NONBLOCK);
	if (pipe_slave_rd_fd < 0) {
		die("Failed to open() the reading end of slave FIFO \"%s\" in non-blocking mode: %m", pipe_slave_path);
	}

	r = fd_make_blocking(pipe_slave_rd_fd);
	if (r < 0) {
		die("Failed to set blocking mode on the reading end of slave FIFO: %m");
	}

	/*
	 * Construct a request.
	 */

	_cleanup_free_ struct Request *request = NULL;
	r = make_request((void**)&request, sizeof(struct Request) - 1 + strlen(file) + 1);
	if (r < 0) {
		die("Failed to make a request buffer: %m");
	}

	request->pid = getpid();
	strcpy(request->path, file);

	/*
	 * Now send the request, open our FIFO for the second time (without blocking on opening)
	 * and do the poll() dance.
	 */

	r = send_request(pipe_master_wr_fd, (struct MessageHeader*)request);
	if (r < 0) {
		die("Failed to send the request: %m");
	}

	struct pollfd fds[] = {
		{
			.fd = pipe_guard_rd_fd,
			.events = POLLIN
		},

		{
			.fd = pipe_slave_rd_fd,
			.events = POLLIN
		}
	};

	r = poll(fds, elementsof(fds), -1);
	if (r < 0) {
		die("Failed to poll() the reading ends of slave and guard FIFOs: %m");
	} else if (r == 0) {
		die("Timed out while trying to poll() the reading ends of slave and guard FIFOs - should not happen.");
	}

	if (fds[0].revents) {
		if (fds[0].revents & POLLHUP) {
			die("[Dead server] The guard FIFO reported hangup.");
		} else {
			die("The guard FIFO reported unexpected events: %hd", fds[0].revents);
		}
	}

	/*
	 * Don't bother checking events for the client FIFO reading end; we'll see what does it have for us.
	 */

	r = fd_cat(pipe_slave_rd_fd, out_fd, 4096); /* one "common" page size worth of a buffer. why not? */
	if (r != RESULT_OK) {
		log("Failed to copy data to the output: %m");
	}

	return 0;
}
