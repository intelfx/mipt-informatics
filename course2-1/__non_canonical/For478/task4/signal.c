#include "../../../common.h"

/*
 * A context (the buffer) for bitwise operations.
 */
struct bitwise_context {
	uint8_t buffer;
};

/*
 * Initialize the bitwise operations buffer for the receiver.
 */
void bitwise_init_rcv(struct bitwise_context *ctx)
{
	assert(ctx);

	ctx->buffer = 1;
}

/*
 * Deinitialize the bitwise operations buffer of the receiver.
 */
int bitwise_cleanup_rcv(struct bitwise_context *ctx)
{
	assert(ctx);

	if (ctx->buffer != 1) {
		int bits_in_buffer = 7 - (__builtin_clz((unsigned)ctx->buffer) - 8 * (sizeof(unsigned) - 1));
		log("Communication error: transmission ends not on a byte boundary: %d bits received out of 8", bits_in_buffer);
		return -1;
	}

	return 0;
}

/*
 * Receive a bit into the buffer, pushing it in from the least significant side.
 *
 * @response_byte: set to the contents of the buffer if a full byte has been received.
 * @response_has_byte: set to `true` if @response_byte has been assigned this time, and set to `false` otherwise.
 */
void bitwise_push_lsb(struct bitwise_context *ctx, bool bit, bool *response_has_byte, uint8_t *response_byte)
{
	bool buffer_full;

	assert(ctx);

	buffer_full = (ctx->buffer & 0x80);

	ctx->buffer <<= 1;
	if (bit) {
		ctx->buffer |= 1;
	}

	if (buffer_full) {
		*response_has_byte = true;
		*response_byte = ctx->buffer;
		ctx->buffer = 1;
	} else {
		*response_has_byte = false;
	}
}

/*
 * Initialize the bitwise operations buffer for the sender.
 */
void bitwise_init_snd(struct bitwise_context *ctx)
{
	assert(ctx);

	ctx->buffer = 0x80;
}

/*
 * Deinitialize the bitwise operations buffer of the sender.
 */
int bitwise_cleanup_snd(struct bitwise_context *ctx)
{
	assert(ctx);

	if (ctx->buffer != 0x80) {
		int bits_in_buffer = 7 - __builtin_ctz((unsigned)ctx->buffer);
		log("Communication error: transmission ends not on a byte boundary: %d bits sent out of 8", bits_in_buffer);
		return -1;
	}

	return 0;
}

bool bitwise_pull(struct bitwise_context *ctx, bool *need_byte)
{
	bool ret;

	assert(ctx);
	assert(need_byte);
	assert(ctx->buffer != 0x80);

	ret = (ctx->buffer & 0x80) ? true : false;
	ctx->buffer <<= 1;
	*need_byte = (ctx->buffer == 0x80);
	return ret;
}

bool bitwise_pull_next_byte(struct bitwise_context *ctx, uint8_t byte)
{
	bool ret;

	assert(ctx);
	assert(ctx->buffer == 0x80);

	ret = (byte & 0x80) ? true : false;
	ctx->buffer = (byte << 1) | 1;
	return ret;
}

/*
 * A receiver context.
 */
struct rcv_context {
	struct bitwise_context accumulator;
	uint8_t byte;
	bool has_byte;
	bool exiting;
} rcv_context;

void rcv_handler(int signal)
{
	switch(signal) {
	case SIGUSR1:
		bitwise_push_lsb(&rcv_context.accumulator, false, &rcv_context.has_byte, &rcv_context.byte);
		break;

	case SIGUSR2:
		bitwise_push_lsb(&rcv_context.accumulator, true, &rcv_context.has_byte, &rcv_context.byte);
		break;

	case SIGTERM:
		rcv_context.exiting = true;
		break;
	}
}

int rcv_main(int out_fd, int snd_pid)
{
	sigset_t sigset_listen_to_server = sigset_inverse_many(SIGUSR1, SIGUSR2, SIGTERM, 0);
	struct sigaction signal_handler;
	bool has_errors = false;
	int r;

	/*
	 * First, configure signal handlers.
	 * We use a common signal handler for all signals we wish to handle and distinguish the signals there.
	 */

	memzero(signal_handler);
	signal_handler.sa_handler = &rcv_handler;
	signal_handler.sa_flags = SA_NODEFER; /* we build the mask by ourselves */

	r = sigaction_many(&signal_handler, SIGUSR1, SIGUSR2, SIGTERM, 0);
	if (r < 0) {
		log("Failed to set up signal handlers: %m");
		goto kill_sender;
	}

	/*
	 * Then, set global variables for our handlers.
	 */

	bitwise_init_rcv(&rcv_context.accumulator);

	/*
	 * Finally, kick the sender and wait for the data.
	 * The handler will terminate us once there is no more data.
	 */

	for (;;) {
		kill(snd_pid, SIGUSR1);
		sigsuspend(&sigset_listen_to_server);

		if (rcv_context.exiting) {
			goto out;
		}

		if (rcv_context.has_byte) {
			rcv_context.has_byte = false;
			r = write(out_fd, &rcv_context.byte, 1);
			if (r < 0) {
				log("Failed to write() one byte to fd %d: %m", out_fd);
				has_errors = 1;
				goto deinit_bitwise;
			}
		}
	}

deinit_bitwise:
	r = bitwise_cleanup_rcv(&rcv_context.accumulator);
	if (r < 0) {
		log("Failed to cleanup bitwise channel: %m");
		has_errors = true;
	}
kill_sender:
	kill(snd_pid, SIGTERM);
out:
	return has_errors;
}

/*
 * A sender context.
 */
struct snd_context {
	bool sending;
	bool exiting;
} snd_context;

void snd_handler(int signal)
{
	switch(signal) {
	case SIGUSR1:
		snd_context.sending = true;
		break;

	case SIGTERM:
		snd_context.exiting = true;
		break;
	}
}

int snd_main(int in_fd, int rcv_pid)
{
	sigset_t sigset_listen_to_client = sigset_inverse_many(SIGUSR1, SIGTERM);
	struct sigaction signal_handler;
	struct bitwise_context accumulator;
	bool has_errors = false;
	bool need_byte = true;
	int r;

	/*
	 * First, configure signal handlers.
	 * We use a common signal handler for all signals we wish to handle and distinguish the signals there.
	 */

	memzero(signal_handler);
	signal_handler.sa_handler = &snd_handler;
	signal_handler.sa_flags = SA_NODEFER; /* we build the mask by ourselves */

	r = sigaction_many(&signal_handler, SIGUSR1, SIGTERM, 0);
	if (r < 0) {
		log("Failed to set up signal handlers: %m");
		goto kill_receiver;
	}

	/*
	 * Then, initialize the bitwise accumulator for sending.
	 */

	bitwise_init_snd(&accumulator);

	/*
	 * Finally, wait for the receiver and push data (if it told us it's not dying and ready to receive).
	 */

	for (;;) {
		sigsuspend(&sigset_listen_to_client);

		if (snd_context.exiting) {
			log("Exiting on request of the client.");
			has_errors = true;
			goto out;
		}

		if (snd_context.sending) {
			bool bit;

			if (need_byte) {
				uint8_t byte;

				r = read(in_fd, &byte, 1);
				if (r == 0) {
					goto kill_receiver;
				} else if (r != 1) {
					log("Failed to read() one byte from fd %d: %m", in_fd);
					has_errors = 1;
					goto kill_receiver;
				}

				need_byte = false;
				bit = bitwise_pull_next_byte(&accumulator, byte);
			} else {
				bit = bitwise_pull(&accumulator, &need_byte);
			}

			kill(rcv_pid, bit ? SIGUSR2 : SIGUSR1);
		}
	}

kill_receiver:
	kill(rcv_pid, SIGTERM);
out:
	r = bitwise_cleanup_snd(&accumulator);
	if (r < 0) {
		has_errors = true;
	}
	close(in_fd);
	return has_errors;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		die("This program takes one argument.");
	}

	/*
	 * Check that we can (and do) open the file specified before making any IPC.
	 */

	const char *file_path = argv[1];
	_cleanup_close_ int file_fd = open(file_path, O_RDONLY);
	if (file_fd < 0) {
		die("Failed to open() file '%s': %m", file_path);
	}

	/*
	 * Ignore SIGCHLD -- let the kernel reap children for us.
	 */
	signal(SIGCHLD, SIG_IGN);

	/*
	 * Block our IPC signals before forking to avoid race conditions during initialization.
	 * The signal masks are preserved across fork().
	 */

	sigset_t sigset_ipc = sigset_many(SIGUSR1, SIGUSR2, SIGTERM, 0);
	sigprocmask(SIG_BLOCK, &sigset_ipc, NULL);

	/*
	 * Finally, fork off the child and specialize.
	 */

	int child_pid = fork();
	if (child_pid == 0) {
		return snd_main(file_fd, getppid());
	} else if (child_pid < 0) {
		die("Failed to fork() off a child: %m");
	} else {
		close(file_fd);
		return rcv_main(STDOUT_FILENO, child_pid);
	}
}
