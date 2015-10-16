#include "common.h"
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define NR_CHILDREN 10

struct msg {
	long mtype;
	int payload;
};

static const size_t msg_size = sizeof(struct msg) - sizeof(((struct msg *)0)->mtype);

void __attribute__((noreturn)) child_main(int queue)
{
	int r;
	ssize_t rszt;

	struct msg msg;
	rszt = msgrcv(queue, &msg, msg_size, getpid(), 0);
	chk(rszt >= 0, "Could not msgrcv() the message from parent: %m");
	chk(rszt == msg_size, "Wrong message size received: %zd bytes of %zu", rszt, msg_size);
	chk(msg.mtype == getpid(), "Wrong message id received: %ld, expected %d", msg.mtype, getpid());

	printf("%d ", msg.payload);
	fflush(stdout);

	msg.mtype = getppid();
	r = msgsnd(queue, &msg, 0, 0);
	chk(r == 0, "Could not msgsnd() the reply: %m");

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int r;
	ssize_t rszt;

	/*
	 * Create an unique message queue which will be shared by all processes.
	 */
	int queue = msgget(IPC_PRIVATE, 0600);
	chk(queue != -1, "Could not create a XSI message queue with msgget(): %m");

	/*
	 * Ignore SIGCHLD to simplify waiting for children.
	 */
	signal(SIGCHLD, SIG_IGN);
	
	int child_pids[NR_CHILDREN];
	int children = 0;
	for (; children < NR_CHILDREN; ++children) {
		int child_pid = fork();
		if (child_pid < 0) {
			log("Could not fork() %d-th child: %m", children+1);
			goto out;
		}

		if (child_pid == 0) {
			child_main(queue);
		}

		child_pids[children] = child_pid;
	}

	for (int i = 0; i < children; ++i) {
		struct msg msg = {
			.mtype = child_pids[i],
			.payload = i
		};

		r = msgsnd(queue, &msg, msg_size, 0);
		if (r != 0) {
			log("Could not msgsnd() message to %d-th child: %m", i);
			goto out;
		}

		rszt = msgrcv(queue, &msg, 0, getpid(), 0);
		if (rszt < 0) {
			log("Could not msgrcv() reply from %d-th child: %m", i);
			goto out;
		}
		if (rszt != 0) {
			log("Wrong message size received: %zd bytes of %zu", rszt, msg_size);
			goto out;
		}
	}

	errno = 0;
	r = wait(NULL);
	children = 0;

out:
	for (int i = 0; i < children; ++i) {
		kill(child_pids[i], SIGTERM);
	}
	msgctl(IPC_RMID, queue, NULL);
}
