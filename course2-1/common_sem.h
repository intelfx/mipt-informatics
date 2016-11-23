#pragma once

#include "common_util.h"

#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_ADJ -1

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

static inline int shm_get_and_attach(int ipc_key, size_t size, int mode, void **result)
{
	/* first, get the existing shared memory segment */
	_cleanup_shm_ int shm = shmget(ipc_key, size, 0);
	void *memory;
	int r;

	if (shm > 0) {
		/* OK */
	} else if (errno != ENOENT) {
		/* failed, and failure is not "inexistent" */
		log("Failed to initially shmget() the shared memory segment: %m");
	} else {
		/* failed and failure is "inexistent", try to create it */
		shm = shmget(ipc_key, size, IPC_CREAT | IPC_EXCL | mode);
		if (shm > 0) {
			/* OK */
		} else if (errno != EEXIST) {
			/* failed and not due to the race */
			log("Failed to shmget(IPC_CREAT) a shared memory segment of %zu bytes: %m", size);
		} else {
			/* lost the race against another shm_get_and_attach(), repeat everything */
			return shm_get_and_attach(ipc_key, size, mode, result);
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

static inline struct sembuf semop_adj(unsigned short sem_num, short sem_adj)
{
	struct sembuf sembuf = {
		.sem_num = sem_num,
		.sem_op = sem_adj,
		.sem_flg = SEM_ADJ /* fake flag, see semop_many() */
	};

	return sembuf;
}

/*
 * A shorthand for issuing a semop() for many operations.
 */

static inline int semop_many(int sem, size_t ops_nr, ...)
{
	struct sembuf *ops = alloca(sizeof(struct sembuf) * ops_nr * 2); /* at most */
	size_t ops_nr_res = 0;

	va_list ap;

	va_start(ap, ops_nr);
	for (size_t i = 0; i < ops_nr; ++i) {
		struct sembuf op = va_arg(ap, struct sembuf);
		if (op.sem_flg != SEM_ADJ) {
			ops[ops_nr_res++] = op;
		} else {
			struct sembuf adjust = semop_entry(op.sem_num, op.sem_op, 0),
			              revert = semop_entry(op.sem_num, -op.sem_op, SEM_UNDO);
			if (op.sem_op > 0) {
				ops[ops_nr_res++] = adjust;
				ops[ops_nr_res++] = revert;
			} else {
				ops[ops_nr_res++] = revert;
				ops[ops_nr_res++] = adjust;
			}
		}
	}
	va_end(ap);

	return semop(sem, ops, ops_nr_res);
}

/*
 * semget()s a semaphore set and initializes it, avoiding races with other instances of sem_get_and_init().
 *
 * We use 0-th semaphore as an inverted mutex for these purposes. It is initialized to 1
 * and its semadj is initialized to -1. values[0] must be 1 and adj_values[0] must be 0
 * unless you know what you are doing.
 */
static inline int sem_get_and_init(int ipc_key, int count, int mode)
{
	/* first, get the existing semaphore */
	_cleanup_sem_ int sem = semget(ipc_key, count, IPC_CREAT | mode);
	int r;

	if (sem >= 0) {
		/*
		 * OK, the initialization dance.
		 * We use 0-th semaphore in an inverted manner:
		 * - semaphores are zero-initialized on creation
		 * - we can wait for a semaphore to become 0 and increment it atomically
		 * - this protects against other instances of sem_get_and_init()
		 * - then we can initialize other semaphores, this won't race
		 */

		int r = semop_many(sem,
		                   2,
		                   semop_entry(0, 0, 0),
		                   semop_entry(0, 1, SEM_UNDO));
		if (r < 0) {
			log("Failed to wait-for-zero and increment the semaphore 0 of set %d: %m", sem);
			return r;
		}

		/* Zero-initialize other semaphores.
		 * We can't use SETALL because it clears semadj values. */
		for (int i = 1; i < count; ++i) {
			r = semctl(sem, i, SETVAL, sem_arg_val(0));
			if (r < 0) {
				log("Failed to semctl(SETVAL) to zero-init semaphore %d of set %d: %m", i, sem);
				return r;
			}
		}
	} else {
		/* failed */
		log("Failed to initially semget() the semaphore set: %m");
	}

	/* all done (or not), avoid cleanup handler */
	r = sem; sem = 0;
	return r;
}
