#include "../common_sem.h"

static const char *ipc_key_file = "ipc";
static const int ipc_key_proj_id = 0;

/* one "common" page size worth of a shm buffer */
#define IPC_SHM_SIZE 4096

enum semaphore_id {
	SEMAPHORE_RCV_MUTEX = 0,
	SEMAPHORE_SND_MUTEX,
	SEMAPHORE_MEMORY,
	SEMAPHORE_DATA,
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

static const struct shared_memory shared_memory_init = {
	.rcv_state = RCV_OK,
	.snd_state = SND_NOT_DONE,
};
