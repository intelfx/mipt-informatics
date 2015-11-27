#include "common.h"

static bool exiting = false;

void exit_signal(int sig)
{
	exiting = true;

	/* repeated delivery of the same signal will result in default reaction
	 * (in case we are screwed) */
	signal(sig, SIG_DFL);
}

int main(int argc, char **argv)
{
	int r;

	if (argc != 1) {
		die_ret("This program expects no arguments.");
	}

	key_t ipc_key = ftok(ipc_key_file, ipc_key_proj_id);
	if (ipc_key < 0) {
		die_ret("Failed to ftok() on file \"%s\" to generate an IPC key: %m", ipc_key_file);
	}

	/*
	 * Initialize the exit handlers.
	 */
	signal(SIGTERM, &exit_signal);
	signal(SIGHUP, &exit_signal);
	signal(SIGINT, &exit_signal);

	/*
	 * Initialize the shared memory and the semaphores.
	 * They are initialized into a state which allows the sender to do its first iteration
	 * and gets receiver into a state where it can wait on the sender results.
	 *
	 * The death detection is also configured for the receiver.
	 */

	_cleanup_shm_ int shm = -1;
	_cleanup_sem_ int sem = -1;
	_cleanup_detach_ struct shared_memory *shared_memory = NULL;

	shm = shm_init(ipc_key, &shared_memory);
	if (shm < 0) {
		die_ret("Failed to initialize the shared memory: %m");
	}

	sem = sem_init(ipc_key, semaphore_adj_values_rcv);
	if (sem < 0) {
		die_ret("Failed to initialize the semaphores for the receiver: %m");
	}

	/*
	 * Now do the main loop.
	 */

	int out_fd = STDOUT_FILENO;

	while(!exiting) {
		r = semop_one(sem, SEMAPHORE_DATA, -1, 0);
		if (r < 0) {
			if (errno == EINTR) {
				continue; /* check for exit condition and repeat */
			} else {
				die_ret("Failed to decrement DATA semaphore: %m");
			}
		}

		switch(shared_memory->snd_state) {
		case SND_OK:
			r = write(out_fd, shared_memory->data, shared_memory->data_amount);
			if (r < 0) {
				shared_memory->rcv_state = RCV_IOERROR;
				die_ret("Failed to write() %zu bytes to fd %d: %m", shared_memory->data_amount, out_fd);
			} else if ((size_t)r != shared_memory->data_amount) {
				shared_memory->rcv_state = RCV_IOERROR;
				die_ret("Short write() %d bytes out of %zu to fd %d: %m", r, shared_memory->data_amount, out_fd);
			}
			break;

		case SND_EOF:
			break; /* we effectively reinitialize the shared memory, V the MEMORY semaphore and begin waiting for the next client. */

		case SND_IOERROR:
			die_ret("Sender has reported an I/O error -- exiting.");

		case SND_NOT_DONE:
			die_ret("Sender didn't do anything in its last iteration -- exiting.");

		default:
			die_ret("Switch error");
		}

		shared_memory->snd_state = SND_NOT_DONE;
		shared_memory->rcv_state = RCV_OK;

		r = semop_one(sem, SEMAPHORE_MEMORY, 1, 0);
		if (r < 0) {
			die_ret("Failed to increment MEMORY semaphore: %m");
		}
	}

	return 0;
}
