#include "../common_util.h"
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct msg {
	long mtype;
};

static const size_t msg_size = 0; /* sizeof(struct msg) - sizeof(((struct msg *)0)->mtype); */

void __attribute__((noreturn)) child_main(int queue, long ordinal)
{
	int r;
	ssize_t rszt;
	long mtype = ordinal + 1;

	struct msg msg;
	rszt = msgrcv(queue, &msg, msg_size, ordinal + 1, 0);
	chk(rszt >= 0, "Could not msgrcv() the message from parent: %m");
	chk(rszt == msg_size, "Wrong message size received: %zd bytes of %zu", rszt, msg_size);
	chk(msg.mtype == mtype, "Wrong message id received: %ld, expected %ld", msg.mtype, mtype);

	printf("%ld ", ordinal);
	fflush(stdout);

	msg.mtype = LONG_MAX;
	r = msgsnd(queue, &msg, 0, 0);
	chk(r == 0, "Could not msgsnd() the reply: %m");

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int r;
	ssize_t rszt;

	if (argc != 2) {
		die("This prorgam expects one argument.");
	}

	long nr_children = strtol_or_die(argv[1]);
	if (nr_children < 1) {
		die("Invalid child process count: %ld", nr_children);
	}

	/*
	 * Create an unique message queue which will be shared by all processes.
	 */
	int queue = msgget(IPC_PRIVATE, 0600);
	chk(queue != -1, "Could not create a XSI message queue with msgget(): %m");

	/*
	 * Ignore SIGCHLD to simplify waiting for children.
	 */
	signal(SIGCHLD, SIG_IGN);
	
	long children = 0;
	for (; children < nr_children; ++children) {
		int child_pid = fork();
		if (child_pid < 0) {
			log("Could not fork() %ld-th child: %m", children);
			goto out;
		}

		if (child_pid == 0) {
			child_main(queue, children);
		}
	}

	for (long i = 0; i < children; ++i) {
		struct msg msg = {
			.mtype = i + 1
		};

		r = msgsnd(queue, &msg, msg_size, 0);
		if (r != 0) {
			log("Could not msgsnd() message to %ld-th child: %m", i);
			goto out;
		}

		rszt = msgrcv(queue, &msg, 0, LONG_MAX, 0);
		if (rszt < 0) {
			log("Could not msgrcv() reply from %ld-th child: %m", i);
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
	msgctl(IPC_RMID, queue, NULL);
}
