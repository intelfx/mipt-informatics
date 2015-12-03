#include "../../../common.h"

/*
 * Domain-specific definitions (shared between server and client)
 */

struct Request
{
	struct MessageHeader hdr;
	int pid;
	char path[1];
} __attribute__((packed));

static inline const char *get_pipe_server_path(void)
{
	return "fifo.master";
}

static inline const char *get_pipe_server_guard_path(void)
{
	return "fifo.master-guard";
}

static inline const char *get_pipe_client_path_by_pid(int pid)
{
	return snprintf_static("fifo.slave-%d", pid);
}

