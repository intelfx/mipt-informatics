#include "../common_sem.h"
#include "common.h"

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
	 * Initialize the shared memory and the semaphores.
	 * They are initialized into a state which allows the sender to do its first iteration
	 * and gets receiver into a state where it can wait on the sender results.
	 *
	 * The death detection is also configured for the receiver.
	 *
	 * We do not remove the semaphore set because other receivers could have
	 * opened it already, blocking on the wait-for-zero.
	 * OTOH, we do remove the shared memory because any waiting receiver process
	 * will be able to re-create it after acquiring the semaphore set.
	 */

	int sem = -1;
	_cleanup_shm_ int shm = -1;
	_cleanup_detach_ struct shared_memory *shared_memory = NULL;

	/*
	 * Get/create the semaphore set and lock out other receivers.
	 */
	sem = sem_get_and_init(ipc_key, _SEMAPHORE_COUNT, 0600);
	if (sem < 0) {
		die_ret("Failed to acquire the semaphores for the receiver: %m");
	}

	/*
	 * Get/create and attach the shared memory and initialize it.
	 */
	shm = shm_get_and_attach(ipc_key, IPC_SHM_SIZE, 0600, (void **)&shared_memory);
	if (shm < 0) {
		die_ret("Failed to initialize the shared memory: %m");
	}

	shared_memory->rcv_state = RCV_OK;
	shared_memory->snd_state = SND_NOT_DONE;

	/*
	 * Here goes the initial state. Explanations available.
	 *
	 * Semaphore values:
	 * [SEMAPHORE_RCV_MUTEX] = 1,  // see sem_get_and_init() -- already at 1
	 * [SEMAPHORE_SND_MUTEX] = 1,  // allow the sender to join
	 * [SEMAPHORE_MEMORY]    = 1,  // memory is ready to accept data
	 * [SEMAPHORE_DATA]      = 0,  // there is no data in the memory at start
	 *
	 * Semadj values:
	 * [SEMAPHORE_RCV_MUTEX] = -1, // see sem_get_and_init() -- already at -1
	 * [SEMAPHORE_SND_MUTEX] = -1, // on death, we disallow any future senders to join
	 * [SEMAPHORE_MEMORY]    = 2,  // on death, we want existing sender to iterate freely
	 */
	r = semop_many(sem,
	               3,
	               semop_entry(SEMAPHORE_SND_MUTEX, 1, SEM_UNDO),
	               semop_entry(SEMAPHORE_MEMORY, 1, 0),
	               semop_adj(SEMAPHORE_MEMORY, 2));
	if (r < 0) {
		log("Failed to set initial values and semadj of the semaphores for the receiver: %m");
		return r;
	}

	/*
	 * Now do the main loop.
	 */

	int out_fd = STDOUT_FILENO;

	for (;;) {
		/*
		 * The DATA semaphore denotes availability of data in the shared memory.
		 *
		 * P the DATA semaphore, waiting for the sender to complete reading of the
		 * next data chunk. The semaphore is V'ed by the sender at the end of its
		 * iteration or in case of an unclean termination (twice).
		 */
		r = semop_many(sem, 1, semop_entry(SEMAPHORE_DATA, -1, 0));
		if (r < 0) {
			die_ret("Failed to decrement DATA semaphore: %m");
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
			goto eof;

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
		 */
		r = semop_many(sem, 1, semop_entry(SEMAPHORE_MEMORY, 1, 0));
		if (r < 0) {
			die_ret("Failed to increment MEMORY semaphore: %m");
		}
	}

eof:

	return 0;
}
