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
	 * 
	 * Note that in sender, we do not remove IPCs on exit, because in our model the receiver is long-lived.
	 */

	int shm = -1;
	int sem = -1;
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

	/*
	 * The SESSION semaphore mutually excludes senders from talking
	 * to the receiver simultaneously.
	 * 
	 * P the SESSION semaphore, waiting for other senders to complete
	 * their transmissions. The semaphore is V'ed by the receiver after
	 * handling the sender termination (either clean or unclean).
	 */
	r = semop_one(sem, SEMAPHORE_SESSION, -1, 0);
	if (r < 0) {
		die_ret("Failed to decrement SESSION semaphore: %m");
	}

	for (;;) {
		/*
		 * The MEMORY semaphore denotes availability of shared memory
		 * for reading data into.
		 * 
		 * P the MEMORY semaphore, waiting for the receiver to complete
		 * processing of the last data chunk. The semaphore is V'ed by the
		 * receiver at the end of its iteration or in case of an unclean termination
		 * (twice).
		 */
		r = semop_one(sem, SEMAPHORE_MEMORY, -1, 0);
		if (r < 0) {
			die_ret("Failed to decrement MEMORY semaphore: %m");
		}

		/*
		 * See if the receiver did actually finish its last iteration.
		 * If this variable is set to RCV_NOT_DONE, it means that the server
		 * has died on the last iteration. If it it set to RCV_OK, it means the opposite.
		 * Then we reset the variable back to RCV_NOT_DONE.
		 * 
		 * The only case to handle is when server dies after setting the variable
		 * to RCV_OK, but before it Vs the MEMORY semaphore and before sender sets it back.
		 * This case is handled by twice-V'ing the MEMORY semaphore on receiver unclean
		 * termination. 
		 */
		switch(shared_memory->rcv_state) {
		case RCV_OK:
			/*
			 * If the receiver is OK (that means, it completed its last iteration at
			 * least to the point of setting rcv_state), read another chunk of data into
			 * memory and set our own status variable.
			 */
			r = read(in_fd, shared_memory->data, DATA_MAX);
			if (r < 0) {
				shared_memory->snd_state = SND_IOERROR;
				die_ret("Failed to read() %zu bytes from fd %d: %m", DATA_MAX, in_fd);
			} else if (r == 0) {
				shared_memory->snd_state = SND_EOF;
				break;
			} else {
				shared_memory->data_amount = r;
				shared_memory->snd_state = SND_OK;
			}
			break;

		case RCV_IOERROR:
			die_ret("Receiver has reported an I/O error -- exiting.");

		case RCV_NOT_DONE:
			die_ret("Receiver didn't do anything in its last iteration -- exiting.");

		default:
			die_ret("Switch error");
		}
		shared_memory->rcv_state = RCV_NOT_DONE;

		/*
		 * The DATA semaphore denotes availability of data in the shared memory.
		 *
		 * V the DATA semaphore, allowing the receiver to begin processing the new
		 * chunk of data. This semaphore is P'ed by the receiver at the beginning of
		 * its iteration.
		 */
		r = semop_one(sem, SEMAPHORE_DATA, 1, 0);
		if (r < 0) {
			die_ret("Failed to increment DATA semaphore: %m");
		}
	}

	/*
	 * At correct exit (after EOF), V the DATA semaphore as well and zero out its semadj -- atomically.
	 */
	r = semop_one_and_adj(sem, SEMAPHORE_DATA, 1, 0, -2);
	if (r < 0) {
		die_ret("Failed to increment DATA semaphore and revert semadj adjustments: %m");
	}

	return 0;
}
