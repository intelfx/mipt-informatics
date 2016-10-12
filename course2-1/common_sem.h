#pragma once

#include "common_util.h"

#include <sys/shm.h>
#include <sys/sem.h>

/*
 * XSI (System V) shared memory and semaphore API operations.
 */

void shm_unget(int shm)
{
	if (shm > 0) {
		int r = shmctl(shm, IPC_RMID, NULL);
		if (r < 0 && errno != EINVAL) {
			log("Failed to shmctl(IPC_RMID) the shared memory segment %d: %m", shm);
		}
	}
}

DEFINE_TRIVIAL_CLEANUP_FUNC(int, shm_unget)
#define _cleanup_shm_ \
		_cleanup_(shm_ungetp)

DEFINE_TRIVIAL_CLEANUP_FUNC_UNSAFE(void *, shmdt)
#define _cleanup_detach_ \
		_cleanup_(shmdtp)

static inline int shm_get_and_attach_slave(int ipc_key, size_t size, void **result)
{
	int shm = shmget(ipc_key, size, 0);
	void *memory;

	if (shm > 0) {
		/* OK */
	} else if (errno != ENOENT) {
		/* failed, and failure is not "inexistent" */
		log("Failed to initially shmget() the shared memory segment: %m");
	} else {
		/* failed and failure is "inexistent", try to create it */
		log("Shared memory segment not created -- sleeping for one second.");
		sleep(1);
		return shm_get_and_attach_slave(ipc_key, size, result);
	}

	if (shm > 0) {
		memory = shmat(shm, NULL, 0);
		if (memory == (void *)-1) {
			log("Failed to shmat() the shared memory segment %d: %m", shm);
			return -1;
		}
		*result = memory;
	}

	/* all done (or not) */
	return shm;
}

static inline int shm_get_and_attach_master(int ipc_key, size_t size, int mode, void **result)
{
	_cleanup_shm_ int shm = shmget(ipc_key, size, IPC_CREAT | mode);
	void *memory;
	int r;

	if (shm > 0) {
		/* OK */
	} else {
		/* failed */
		log("Failed to shmget(IPC_CREAT) a shared memory segment of %zu bytes: %m", size);
	}

	if (shm > 0) {
		memory = shmat(shm, NULL, 0);
		if (memory == (void *)-1) {
			log("Failed to shmat() the shared memory segment %d: %m", shm);
			return -1;
		}
		*result = memory;
	}

	/* all done (or not), avoid cleanup handler */
	r = shm; shm = 0;
	return r;
}

static inline int shm_get_and_attach(int ipc_key, size_t size, int mode, bool *shm_created, void **result)
{
	/* first, get the existing shared memory segment */
	_cleanup_shm_ int shm = shmget(ipc_key, size, 0);
	void *memory;
	int r;

	if (shm > 0) {
		/* OK */
		*shm_created = false;
	} else if (errno != ENOENT) {
		/* failed, and failure is not "inexistent" */
		log("Failed to initially shmget() the shared memory segment: %m");
	} else {
		/* failed and failure is "inexistent", try to create it */
		shm = shmget(ipc_key, size, IPC_CREAT | IPC_EXCL | mode);
		if (shm > 0) {
			*shm_created = true;
		} else if (errno != EEXIST) {
			/* failed and not due to the race */
			log("Failed to shmget(IPC_CREAT) a shared memory segment of %zu bytes: %m", size);
		} else {
			/* lost the race against another shm_get_and_attach(), repeat everything */
			return shm_get_and_attach(ipc_key, size, mode, shm_created, result);
		}
	}

	if (shm > 0) {
		memory = shmat(shm, NULL, 0);
		if (memory == (void *)-1) {
			log("Failed to shmat() the shared memory segment %d: %m", shm);
			return -1;
		}
		*result = memory;
	}

	/* all done (or not), avoid cleanup handler */
	r = shm; shm = 0;
	return r;
}

void sem_unget(int sem)
{
	if (sem > 0) {
		int r = semctl(sem, 0, IPC_RMID);
		if (r < 0 && errno != EINVAL) {
			log("Failed to semctl(IPC_RMID) the semaphore set %d: %m", sem);
		}
	}
}

DEFINE_TRIVIAL_CLEANUP_FUNC(int, sem_unget)
#define _cleanup_sem_ \
		_cleanup_(sem_ungetp)

union semun {
	int val;
	struct semid_ds *ds;
	unsigned short *array;
};

static inline union semun sem_arg_array(unsigned short *sem_array)
{
	union semun sem_arg = {
		.array = sem_array
	};
	return sem_arg;
}

static inline union semun sem_arg_ds(struct semid_ds *sem_ds)
{
	union semun sem_arg = {
		.ds = sem_ds
	};
	return sem_arg;
}

static inline union semun sem_arg_val(int sem_val)
{
	union semun sem_arg = {
		.val = sem_val
	};
	return sem_arg;
}

static inline int sem_wait_for_otime(int sem)
{
	int r;

	for (;;) {
		struct semid_ds sem_ds;

		r = semctl(sem, 0, IPC_STAT, sem_arg_ds(&sem_ds));
		if (r < 0) {
			log("Failed to semctl(IPC_STAT) the semaphore set %d: %m", sem);
			return r;
		}
		if (sem_ds.sem_otime != 0) {
			break;
		}
		log("Semaphore set created and not initialized -- sleeping for one second.");
		sleep(1);
	}

	return r;
}

/*
 * Creates a struct sembuf.
 */

static inline struct sembuf semop_entry(unsigned short sem_num, short sem_op, short sem_flg)
{
	struct sembuf sembuf = {
		.sem_num = sem_num,
		.sem_op = sem_op,
		.sem_flg = sem_flg
	};

	return sembuf;
}

/*
 * A shorthand for issuing a semop() for many operations.
 */

static inline int semop_many(int sem, size_t ops_nr, ...)
{
	struct sembuf *ops = alloca(sizeof(struct sembuf) * ops_nr);
	va_list ap;

	va_start(ap, ops_nr);
	for (size_t i = 0; i < ops_nr; ++i) {
		ops[i] = va_arg(ap, struct sembuf);
	}
	va_end(ap);

	return semop(sem, ops, ops_nr);
}

/*
 * A shorthand for issuing a semop() for one operation and arbitrary adjustment of semadj.
 */

static inline int semop_one_and_adj(int sem, unsigned short sem_num, short sem_op, short sem_flg, short adj)
{
	struct sembuf op = semop_entry(sem_num, sem_op, sem_flg);

	if (adj == 0) {
		return semop_many(sem, 1, op);
	} else {
		struct sembuf adjust = semop_entry(sem_num, adj, 0),
		              revert = semop_entry(sem_num, -adj, SEM_UNDO);

		if (adj > 0) {
			return semop_many(sem, 3, op, adjust, revert);
		} else {
			return semop_many(sem, 3, op, revert, adjust);
		}
	}
}

/*
 * Adjusts the semadj values of a semaphore set.
 *
 * For positive adjustments, this is done by performing the specified number of V operations
 * and then the same number of P operations with SEM_UNDO specified, all in the same semop() call
 * (hence atomically).
 *
 * Returns 1 if any adjustments were actually made, 0 if all values were zero, -1 (with errno set) on errors.
 */
static inline int sem_adj_many(int sem, int count, const short *adj_values)
{
	struct sembuf *sem_adj_ops;
	size_t sem_adj_ops_nr = 0;
	int r;

	assert(count > 0);

	/* we need `count * 2` operations at most */
	sem_adj_ops = alloca(sizeof(struct sembuf) * count * 2);

	for (unsigned short i = 0; i < count; ++i) {
		struct sembuf adjust = semop_entry(i, adj_values[i], 0),
		              revert = semop_entry(i, -adj_values[i], SEM_UNDO);

		if (adj_values[i] > 0) {
			sem_adj_ops[sem_adj_ops_nr++] = adjust; /* V */
			sem_adj_ops[sem_adj_ops_nr++] = revert; /* P */
		} else if (adj_values[i] < 0) {
			sem_adj_ops[sem_adj_ops_nr++] = revert; /* V */
			sem_adj_ops[sem_adj_ops_nr++] = adjust; /* P */
		}
	}

	if (sem_adj_ops_nr) {
		r = semop(sem, sem_adj_ops, sem_adj_ops_nr);
		if (r < 0) {
			return r;
		}
		return 1;
	} else {
		return 0;
	}
}

static inline int sem_get_and_init_slave(int ipc_key, int count, const short *adj_values)
{
	int sem = semget(ipc_key, count, 0);

	if (sem > 0) {
		/* OK, wait for it to be initialized */
		int r = sem_wait_for_otime(sem);
		if (r < 0) {
			log("Failed to wait for initialization of the semaphore set %d: %m", sem);
			return r;
		}

		/* ...and set our own semadj values */
		r = sem_adj_many(sem, count, adj_values);
		if (r < 0) {
			log("Failed to set initial semadj values of the semaphore set %d: %m", sem);
			return r;
		}
	} else if (errno != ENOENT) {
		/* failed, and failure is not "inexistent" */
		log("Failed to semget() the semaphore set: %m");
	} else {
		/* failed and failure is "inexistent", wait */
		log("Semaphore set not created -- sleeping for one second.");
		sleep(1);
		return sem_get_and_init_slave(ipc_key, count, adj_values);
	}

	return sem;
}

static inline int sem_get_and_init_master(int ipc_key, int count, int mode, const unsigned short *values, const short *adj_values)
{
	_cleanup_sem_ int sem = semget(ipc_key, count, IPC_CREAT | mode);
	int r;

	if (sem > 0) {
		/* OK, the initialization dance.
		 *
		 * - semctl(SETALL) does not set otime.
		 * - our method of setting semadj values does set otime, but only if there was something to set.
		 * - semop(..., { .sem_num = ..., .sem_op = 0, .sem_flg = IPC_NOWAIT }, 1) _hopefully_ sets otime
		 *   and does nothing else (if I read the man correctly).
		 *
		 * So let's do everything outlined here. First -- SETALL. */
		r = semctl(sem, 0, SETALL, sem_arg_array((unsigned short *)values));
		if (r < 0) {
			log("Failed to call semctl(SETALL) to initialize the semaphore set %d: %m", sem);
			return r;
		}

		/* then the initial semadj values (and set otime coincidentally if there are any nonzero initial values) */
		r = sem_adj_many(sem, count, adj_values);
		if (r < 0) {
			log("Failed to set initial semadj values of the semaphore set %d: %m", sem);
			return r;
		} else if (r == 0) {
			/* there were no semadj initial values to set, let's perform a dummy operation to set otime. */
			r = semop_many(sem, 1, semop_entry(0, 0, IPC_NOWAIT));
			if (r < 0) {
				log("Failed to do a dummy semop() on the semaphore set %d: %m", sem);
				return r;
			}
		}
	} else {
		/* failed */
		log("Failed to semget(IPC_CREAT) the semaphore set: %m");
	}

	/* all done (or not), avoid cleanup handler */
	r = sem; sem = 0;
	return r;
}

static inline int sem_get_and_init(int ipc_key, int count, int mode, const unsigned short *values, const short *adj_values)
{
	/* first, get the existing semaphore */
	_cleanup_sem_ int sem = semget(ipc_key, count, 0);
	int r;

	if (sem > 0) {
		/* OK, wait for it to be initialized */
		int r = sem_wait_for_otime(sem);
		if (r < 0) {
			log("Failed to wait for initialization of the semaphore set %d: %m", sem);
			return r;
		}

		/* ...and set our own semadj values */
		r = sem_adj_many(sem, count, adj_values);
		if (r < 0) {
			log("Failed to set initial semadj values of the semaphore set %d: %m", sem);
			return r;
		}
	} else if (errno != ENOENT) {
		/* failed, and failure is not "inexistent" */
		log("Failed to initially semget() the semaphore set: %m");
	} else {
		/* failed and failure is "inexistent", try to create it */
		sem = semget(ipc_key, count, IPC_CREAT | IPC_EXCL | mode);
		if (sem > 0) {
			/* OK, the initialization dance.
			 *
			 * - semctl(SETALL) does not set otime.
			 * - our method of setting semadj values does set otime, but only if there was something to set.
			 * - semop(..., { .sem_num = ..., .sem_op = 0, .sem_flg = IPC_NOWAIT }, 1) _hopefully_ sets otime
			 *   and does nothing else (if I read the man correctly).
			 *
			 * So let's do everything outlined here. First -- SETALL. */
			r = semctl(sem, 0, SETALL, sem_arg_array((unsigned short *)values));
			if (r < 0) {
				log("Failed to call semctl(SETALL) to initialize the semaphore set %d: %m", sem);
				return r;
			}

			/* then the initial semadj values (and set otime coincidentally if there are any nonzero initial values) */
			r = sem_adj_many(sem, count, adj_values);
			if (r < 0) {
				log("Failed to set initial semadj values of the semaphore set %d: %m", sem);
				return r;
			} else if (r == 0) {
				/* there were no semadj initial values to set, let's perform a dummy operation to set otime. */
				r = semop_many(sem, 1, semop_entry(0, 0, IPC_NOWAIT));
				if (r < 0) {
					log("Failed to do a dummy semop() on the semaphore set %d: %m", sem);
					return r;
				}
			}
		} else if (errno != EEXIST) {
			/* failed and not due to the race */
			log("Failed to semget(IPC_CREAT) the semaphore set: %m");
		} else {
			/* lost the race against another sem_get_and_init(), repeat everything */
			return sem_get_and_init(ipc_key, count, mode, values, adj_values);
		}
	}

	/* all done (or not), avoid cleanup handler */
	r = sem; sem = 0;
	return r;
}
