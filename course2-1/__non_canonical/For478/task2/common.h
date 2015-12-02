#include "../../../common.h"

static const char *ipc_key_file = "ipc";
static const int ipc_key_proj_id = 0;

/* one "common" page size worth of a shm buffer */
#define IPC_SHM_SIZE 4096

enum semaphore_id {
	SEMAPHORE_MEMORY,
	SEMAPHORE_DATA,
	SEMAPHORE_SESSION,
	_SEMAPHORE_COUNT
};

enum rcv_state {
	RCV_NOT_DONE,
	RCV_OK,
	RCV_IOERROR
};

enum snd_state {
	SND_NOT_DONE,
	SND_OK,
	SND_EOF,
	SND_IOERROR
};

struct shared_memory {
	enum rcv_state rcv_state;
	enum snd_state snd_state;
	size_t data_amount;
	char data[1];
};

#define DATA_MAX (sizeof(struct shared_memory) - 1)

/*
 * Here goes the initial state. Explanations available.
 */

static const unsigned short semaphore_init_values[_SEMAPHORE_COUNT] = {
	[SEMAPHORE_MEMORY] = 1, /* memory is ready to accept data */
	[SEMAPHORE_DATA] = 0, /* there is no data in the memory at start */
	[SEMAPHORE_SESSION] = 1, /* session is ready to begin */
};

static const short semaphore_adj_values_rcv[_SEMAPHORE_COUNT] = {
	[SEMAPHORE_MEMORY] = 2, /* on death, receiver wants sender to iterate freely */
};

static const short semaphore_adj_values_snd[_SEMAPHORE_COUNT] = {
	[SEMAPHORE_DATA] = 2, /* on death, sender wants receiver to iterate freely */
};

static const struct shared_memory shared_memory_init = {
	.rcv_state = RCV_OK, /* together with initial value for MEMORY semaphore, this allows the sender to perform its first iteration */
	.snd_state = SND_NOT_DONE,
};

static inline int sem_init_master(int ipc_key, const short *sem_adj_values)
{
	return sem_get_and_init_master(ipc_key, _SEMAPHORE_COUNT, 0600, semaphore_init_values, sem_adj_values);
}

static inline int shm_init_master(int ipc_key, struct shared_memory **result)
{
	int shm = shm_get_and_attach_master(ipc_key, IPC_SHM_SIZE, 0600, (void **)result);

	if (shm > 0) {
		**result = shared_memory_init;
	}

	return shm;
}

static inline int sem_init_slave(int ipc_key, const short *sem_adj_values)
{
	return sem_get_and_init_slave(ipc_key, _SEMAPHORE_COUNT, sem_adj_values);
}

static inline int shm_init_slave(int ipc_key, struct shared_memory **result)
{
	return shm_get_and_attach_slave(ipc_key, IPC_SHM_SIZE, (void **)result);
}
