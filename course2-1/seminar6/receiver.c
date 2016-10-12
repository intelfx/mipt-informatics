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

	shm = shm_init_master(ipc_key, IPC_EXCL, &shared_memory);
	if (shm < 0) {
		die_ret("Failed to initialize the shared memory: %m");
	}

	sem = sem_init_master(ipc_key, IPC_EXCL, semaphore_adj_values_rcv);
	if (sem < 0) {
		die_ret("Failed to initialize the semaphores for the receiver: %m");
	}

	/*
	 * Now do the main loop.
	 */

	int out_fd = STDOUT_FILENO;

	while(!exiting) {
		bool has_eof = false;

		/*
		 * The DATA semaphore denotes availability of data in the shared memory.
		 *
		 * P the DATA semaphore, waiting for the sender to complete reading of the
		 * next data chunk. The semaphore is V'ed by the sender at the end of its
		 * iteration or in case of an unclean termination (twice).
		 */
		r = semop_many(sem, 1, semop_entry(SEMAPHORE_DATA, -1, 0));
		if (r < 0) {
			/*
			 * If a signal interrupts semop(), it returns with EINTR.
			 * We use this behavior to implement "correct exiting" on receipt of common
			 * termination signals -- the handler sets `exiting`, and we recheck it
			 * when we get EINTR (rechecking is done because EINTR can happen due to any
			 * signal, not only one of explicitly handled).
			 * 
			 * We don't bother doing any special handling of exit, because IPCs are anyway
			 * destroyed, and the sender will get EIDRM on next semaphore operation.
			 */
			if (errno == EINTR) {
				continue; /* check for exit condition and repeat */
			} else {
				die_ret("Failed to decrement DATA semaphore: %m");
			}
		}

		/*
		 * See if the sender did actually finish its last iteration.
		 * If this variable is set to SND_NOT_DONE, it means that the sender
		 * has died on the last iteration. If it it set to SND_OK, it means the opposite.
		 * Then we reset the variable back to SND_NOT_DONE.
		 * 
		 * The only case to handle is when sender dies after setting the variable
		 * to SND_OK, but before it Vs the DATA semaphore and before receiver sets it back to SND_NOT_DONE.
		 * This case is handled by twice-V'ing the DATA semaphore on sender unclean
		 * termination. 
		 */
		switch(shared_memory->snd_state) {
		case SND_OK:
			/*
			 * If the sender is OK (that means, it completed its last iteration at least
			 * to the point of setting snd_state), write the data to file.
			 */
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
			/*
			 * If the sender is done, V the SESSION semaphore in addition to whatever we typically do
			 * and begin waiting for the next client.
			 */
			has_eof = true;
			break;

		case SND_IOERROR:
			die_ret("Sender has reported an I/O error -- exiting.");

		case SND_NOT_DONE:
			die_ret("Sender didn't do anything in its last iteration -- exiting.");

		default:
			die_ret("Switch error");
		}
		shared_memory->rcv_state = RCV_OK;
		shared_memory->snd_state = SND_NOT_DONE;

		/*
		 * The MEMORY semaphore denotes availability of shared memory
		 * for reading data into.
		 *
		 * V the MEMORY semaphore, allowing the sender to begin reading the new
		 * chunk of data into memory. This semaphore is P'ed by the sender at the beginning of
		 * its iteration.
		 * 
		 * If, on the other hand, the receiver did just finish its work and should've exited,
		 * also V the SESSION semaphore, allowing the next sender in.
		 * This is not done automatically at sender termination to allow us to see the EOF condition
		 * before the next sender takes in and overwrites it with data and sets the OK status.
		 */
		if (has_eof) {
			r = semop_many(sem, 1, semop_entry(SEMAPHORE_SESSION, 1, 0));
			if (r < 0) {
				die_ret("Failed to increment SESSION semaphore: %m");
			}
		}
		r = semop_many(sem, 1, semop_entry(SEMAPHORE_MEMORY, 1, 0));
		if (r < 0) {
			die_ret("Failed to increment MEMORY semaphore: %m");
		}
	}

	return 0;
}
