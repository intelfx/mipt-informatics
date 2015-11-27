#include "common.h"

int main(int argc, char **argv)
{
	int r;

	if (argc != 2) {
		die_ret("This program expects one argument.");
	}

	const char *in_file = argv[1];
	int in_fd = open(in_file, O_RDONLY);
	if (in_fd < 0) {
		die_ret("Failed to open() input file \"%s\": %m", in_file);
	}

	key_t ipc_key = ftok(ipc_key_file, ipc_key_proj_id);
	if (ipc_key < 0) {
		die_ret("Failed to ftok() on file \"%s\" to generate an IPC key: %m", ipc_key_file);
	}

	/*
	 * Initialize the shared memory and the semaphores.
	 * They are initialized into a state which allows the sender to do its first iteration
	 * and gets receiver into a state where it can wait on the sender results.
	 * 
	 * The death detection is also configured for the sender.
	 */

	_cleanup_shm_ int shm = -1;
	_cleanup_sem_ int sem = -1;
	_cleanup_detach_ struct shared_memory *shared_memory = NULL;

	shm = shm_init(ipc_key, &shared_memory);
	if (shm < 0) {
		die_ret("Failed to initialize the shared memory: %m");
	}

	sem = sem_init(ipc_key, semaphore_adj_values_snd);
	if (sem < 0) {
		die_ret("Failed to initialize the semaphores for the sender: %m");
	}

	/*
	 * Now do the main loop.
	 */

	bool exiting = false;

	while (!exiting) {
		r = semop_one(sem, SEMAPHORE_MEMORY, -1, 0);
		if (r < 0) {
			die_ret("Failed to decrement MEMORY semaphore: %m");
		}

		switch(shared_memory->rcv_state) {
		case RCV_OK:
			break;

		case RCV_IOERROR:
			die_ret("Receiver has reported an I/O error -- exiting.");

		case RCV_NOT_DONE:
			die_ret("Receiver didn't do anything in its last iteration -- exiting.");

		default:
			die_ret("Switch error");
		}

		shared_memory->rcv_state = RCV_NOT_DONE;

		r = read(in_fd, shared_memory->data, DATA_MAX);
		if (r < 0) {
			shared_memory->snd_state = SND_IOERROR;
			die_ret("Failed to read() %zu bytes from fd %d: %m", DATA_MAX, in_fd);
		} else if (r == 0) {
			shared_memory->snd_state = SND_EOF;
			exiting = true;
		} else {
			shared_memory->data_amount = r;
			shared_memory->snd_state = SND_OK;
		}

		r = semop_one(sem, SEMAPHORE_DATA, 1, 0);
		if (r < 0) {
			die_ret("Failed to increment DATA semaphore: %m");
		}
	}

	return 0;
}
