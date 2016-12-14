#include "../common_util.h"
#include "../common_fileops.h"
#include <sys/epoll.h>
#include <time.h>
#include <bsd/unistd.h>

#ifdef DEBUG
# define dbg(...) log(__VA_ARGS__)
#else
# define dbg(...) do { } while (0)
#endif

static const size_t BUF_MIN = 4*1024+1, BUF_MAX = 16*1024;
static const uint32_t EPOLLFAKE = 0x80000000;

size_t rand_buffer_size()
{
	return BUF_MIN + (rand() % (BUF_MAX - BUF_MIN + 1));
}

void kill_all_and_die(int code)
{
	int r;

	r = kill(0, SIGKILL);
	if (r < 0) {
		log("[parent] Failed to kill(0, SIGKILL): %m");
	}

	exit(code);
}

void sigchld_handler(int sig, siginfo_t *info, void *ctx)
{
	if (WIFEXITED(info->si_status) && WEXITSTATUS(info->si_status) == 0) {
		log("[parent] child %d terminated normally", info->si_pid);
	} else {
		log("[parent] child %d terminated with status %d -- killing process group and exiting",
		    info->si_pid,
		    info->si_status);

		kill_all_and_die(info->si_status);
	}
}

ssize_t child_cat_fd_iter(size_t child_id, int fd_in, int fd_out, void *buf, size_t buf_size)
{
	ssize_t rd_size = read(fd_in, buf, buf_size);
	if (rd_size < 0) {
		die_ret("[child %zu] Failed to read() %zu bytes from the input stream: %m",
		        child_id, buf_size);
	}
	if (rd_size == 0) {
		log("[child %zu] Got EOF on read() from the input stream, exiting",
		    child_id);
		return 0;
	}

	ssize_t wr_size = write(fd_out, buf, rd_size);
	if (wr_size < 0) {
		die_ret("[child %zu] Failed to write() %zu bytes to the output stream: %m",
		        child_id, rd_size);
	}
	if (wr_size != rd_size) {
		die_ret("[child %zu] Failed to write() %zu bytes to the output stream, wrote %zu bytes",
		        child_id, rd_size, wr_size);
	}

	return wr_size;
}

int child_main(size_t child_id, int fd_in, int fd_out)
{
	srand(time(NULL) ^ child_id);

	log("[child %zu] started with PID %d", child_id, getpid());

	size_t buf_size = rand_buffer_size();
	log("[child %zu] buffer size = %zu", child_id, buf_size);

	void *buf = malloc(buf_size);

	for (;;) {
		ssize_t r = child_cat_fd_iter(child_id, fd_in, fd_out, buf, buf_size);
		if (r <= 0) {
			return r;
		}
	}
}

int epoll_event_sort_ascending(const void *arg_1, const void *arg_2)
{
	const struct epoll_event *e_1 = arg_1, *e_2 = arg_2;

	if (e_2->data.ptr < e_1->data.ptr) {
		return -1;
	} else if (e_2->data.ptr > e_2->data.ptr) {
		return 1;
	} else {
		return 0;
	}
}

bool consume_flag(uint32_t *arg, uint32_t flag)
{
	if (*arg & flag) {
		*arg ^= flag;
		return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	int r;

	srand(time(NULL));

	if (argc != 3) {
		die("[parent] This program expects two arguments.");
	}

	unsigned long child_nr = strtoul_or_die(argv[1]);

	const char *in_file = argv[2];
	int in_fd = open(in_file, O_RDONLY);
	if (in_fd < 0) {
		die("[parent] Failed to open() input file \"%s\": %m", in_file);
	}

	struct sigaction sigchld = {
		.sa_sigaction = sigchld_handler,
		.sa_flags = SA_SIGINFO | SA_RESETHAND
	};
	r = sigaction(SIGCHLD, &sigchld, NULL);
	if (r < 0) {
		die("[parent] Failed to sigaction(SIGCHLD): %m");
	}

	r = setpgid(0, 0);
	if (r < 0) {
		die("[parent] Failed to setpgid(0, 0) to create a new process group: %m");
	}

	int epoll_fd = epoll_create1(0);
	if (epoll_fd < 0) {
		die("[parent] Failed to epoll_create1(): %m");
	}

	/* i-th buffer is to i-th child, (N+1)-th buffer is to output */
	struct buffer {
		/*
		 * wr_*: where to write from buffer (pipe to this child or the output)
		 * rd_*: where to read to buffer (pipe from previous child or the input)
		 */
		int wr_fd, rd_fd;
		struct epoll_event wr_event, rd_event;

		size_t buf_size;
		size_t begin, end;
		char *buf;
	} *buffers = malloc(sizeof(struct buffer) * (child_nr + 1));

	/* initialize common fields of all buffers */
	for (size_t i = 0; i < child_nr + 1; ++i) {
		struct buffer *b = &buffers[i];

		b->buf_size = rand_buffer_size();
		log("[parent] buffer size for child %zu = %zu", i, b->buf_size);

		b->begin = b->end = 0;
		b->buf = (char *)malloc(b->buf_size);
		if (b->buf == NULL) {
			die("[parent] Failed to malloc() %zu bytes: %m", b->buf_size);
		}

		b->rd_event.events = EPOLLIN;
		b->rd_event.data.ptr = &b->rd_event;
		b->wr_event.events = EPOLLOUT;
		b->wr_event.data.ptr = &b->wr_event;
	}

	log("[parent] buffers initialized");

	/* create children and their pipes */
	for (size_t i = 0; i < child_nr; ++i) {
		int pipe_to_client[2], pipe_from_client[2];

		r = pipe(pipe_to_client);
		if (r < 0) {
			die_out("[parent] Failed to pipe(): %m");
		}

		r = pipe(pipe_from_client);
		if (r < 0) {
			die_out("[parent] Failed to pipe(): %m");
		}

		r = fd_make_non_blocking(pipe_to_client[1]);
		if (r < 0) {
			die_out("[parent] Failed to switch a writing end to non-blocking mode: %m");
		}

		r = fd_make_non_blocking(pipe_from_client[0]);
		if (r < 0) {
			die_out("[parent] Failed to switch a reading end to non-blocking mode: %m");
		}

		buffers[i  ].wr_fd = pipe_to_client[1];
		buffers[i+1].rd_fd = pipe_from_client[0];

		log("[parent] starting child %zu", i);

		pid_t child_pid = fork();
		if (child_pid < 0) {
			die_out("[parent] Failed to fork(): %m");
		}
		if (child_pid == 0) {
			/* reopen pipes as stdin/stdout and close all other fds */
			close(STDIN_FILENO);
			r = dup2(pipe_to_client[0], STDIN_FILENO);
			if (r < 0) {
				die("[child %zu] Failed to dup2() input pipe fd %d as stdin: %m",
				    i,
				    pipe_to_client[0]);
			}

			close(STDOUT_FILENO);
			r = dup2(pipe_from_client[1], STDOUT_FILENO);
			if (r < 0) {
				die("[child %zu] Failed to dup2() output pipe fd %d as stdout: %m",
				    i,
				    pipe_from_client[1]);
			}

			closefrom(STDERR_FILENO + 1);

			r = child_main(i, STDIN_FILENO, STDOUT_FILENO);
			exit(r);
		}

		/* close fds of the child */
		close(pipe_to_client[0]);
		close(pipe_from_client[1]);
	}

	log("[parent] children started");

	/* initialize epoll */
	for (size_t i = 0; i < child_nr + 1; ++i) {
		struct buffer *b = &buffers[i];

		if (i != 0) {
			r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, b->rd_fd, &b->rd_event);
			if (r < 0) {
				die_out("[parent] Failed to epoll_ctl(EPOLL_CTL_ADD): %m");
			}
		}

		if (i != child_nr) {
			r = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, b->wr_fd, &b->wr_event);
			if (r < 0) {
				die_out("[parent] Failed to epoll_ctl(EPOLL_CTL_ADD): %m");
			}
		}
	}

	log("[parent] epoll initialized");

	/* initialize terminal connections (no polling on these) */
	buffers[0].rd_fd = in_fd;
	buffers[0].rd_event.events = EPOLLIN;
	buffers[child_nr].wr_fd = STDOUT_FILENO;
	buffers[child_nr].wr_event.events = EPOLLOUT;

	/*
	 * Now the main loop.
	 * For each connection, we would like to see at the same time whether
	 * the input is ready-in and/or hanged up, whether the buffer holds any data
	 * and whether the output is ready-out.
	 * Moreover, we want to avoid scanning all existing connections in each
	 * iteration of the loop. Finally, we'd like to scan events starting from the
	 * tail.
	 * Hence, because it's C and we have no STL, we sort the epoll events by connection
	 * (actually, by user-data pointer value) in ascending order and then
	 * coalesce them into temporary per-connection structures.
	 */

	size_t epoll_event_nr_max = 1 + 2 /* room for fake events */;
	struct epoll_event *epoll_events = malloc(sizeof(struct epoll_event) * epoll_event_nr_max);

	size_t buffer_event_nr_max = child_nr;
	struct buffer_event {
		struct buffer *buffer;
		bool rd_seen : 1,
		     wr_seen : 1,
		     rd_ready_in : 1,
		     wr_ready_out : 1,
		     rd_hang_up : 1,
		     rd_have_errors : 1,
		     wr_have_errors : 1,
		     rd_non_pollable : 1,
		     wr_non_pollable : 1;
	} *buffer_events = malloc(sizeof(struct buffer_event) * buffer_event_nr_max);

	/* do the main loop */
	for (;;) {
		r = epoll_wait(epoll_fd, epoll_events, epoll_event_nr_max - 2 /* room for fake events */, -1);
		if (r < 0) {
			die_out("Failed to epoll_wait(): %m");
		}

		size_t epoll_event_nr = r;
		dbg("[parent] epoll_wait() returned %zu events", epoll_event_nr);
		if (epoll_event_nr == 0) {
			continue;
		}

		/* inject fake epoll events for reading from input and writing to the output
		 * (if these fds are not closed) */
		assert(epoll_event_nr + 2 <= epoll_event_nr_max);

		if (buffers[0].rd_fd > 0) {
			epoll_events[epoll_event_nr].events = EPOLLFAKE;
			epoll_events[epoll_event_nr].data.ptr = &buffers[0].rd_event;
			++epoll_event_nr;
		}

		if (buffers[child_nr].wr_fd > 0) {
			epoll_events[epoll_event_nr].events = EPOLLFAKE;
			epoll_events[epoll_event_nr].data.ptr = &buffers[child_nr].wr_event;
			++epoll_event_nr;
		}

		/* sort the epoll events */
		qsort(epoll_events, epoll_event_nr, sizeof(epoll_events[0]), &epoll_event_sort_ascending);

		/* coalesce the epoll events into "buffer events" */
		struct buffer_event *be_last = buffer_events;

		for (size_t i = 0; i < epoll_event_nr; ++i) {
			/* the epoll_event returned by epoll_wait() */
			struct epoll_event *ee = epoll_events + i;

			/* the epoll_event pointed to by the user-data ptr */
			struct epoll_event *ee_orig = ee->data.ptr;

			/* whether this is wr_event */
			bool ee_is_wr = !!(ee_orig->events & EPOLLOUT);

			/* recover pointer to the buffer */
			struct buffer *ee_buffer = ee_is_wr
			                         ? container_of(ee_orig, struct buffer, wr_event)
			                         : container_of(ee_orig, struct buffer, rd_event);

			dbg("[parent] epoll_event %zu: buffer %zu fd %s events 0x%08x",
			    i,
			    ee_buffer - buffers,
			    ee_is_wr ? "write" : "read",
			    ee->events);

			/* advance destination pointer if needed */
			if (i == 0) {
				/* first iteration */
				memzero(*be_last);
				be_last->buffer = ee_buffer;
			} else if (be_last->buffer != ee_buffer) {
				/* next buffer */
				++be_last;
				memzero(*be_last);
				be_last->buffer = ee_buffer;
			}

			dbg("[parent] epoll_event %zu: -> buffer_event %zu",
			    i,
			    be_last - buffer_events);

			/* store the event */
			if (ee_is_wr) {
				be_last->wr_seen = 1;
				be_last->wr_ready_out = consume_flag(&ee->events, EPOLLOUT);
				be_last->wr_have_errors = consume_flag(&ee->events, EPOLLERR);
				be_last->wr_non_pollable = consume_flag(&ee->events, EPOLLFAKE);
				dbg("[parent] epoll_event %zu: wr_seen = 1, wr_ready_out = %d, wr_have_errors = %d",
				    i,
				    be_last->wr_ready_out,
				    be_last->wr_have_errors);
			} else {
				be_last->rd_seen = 1;
				be_last->rd_ready_in = consume_flag(&ee->events, EPOLLIN);
				be_last->rd_hang_up = consume_flag(&ee->events, EPOLLHUP);
				be_last->rd_have_errors = consume_flag(&ee->events, EPOLLERR);
				be_last->rd_non_pollable = consume_flag(&ee->events, EPOLLFAKE);
				dbg("[parent] epoll_event %zu: rd_seen = 1, rd_ready_in = %d, rd_hang_up = %d, rd_have_errors = %d",
				    i,
				    be_last->rd_ready_in,
				    be_last->rd_hang_up,
				    be_last->rd_have_errors);
			}

			if (ee->events != 0) {
				die_out("[parent] epoll_event %zu: unconsumed flags for %s fd!",
				        i,
				        ee_is_wr ? "write" : "read");
			}
		}

		size_t buffer_event_nr = be_last - buffer_events + 1;
		dbg("[parent] coalesced into %zu events", buffer_event_nr);

		/* process coalesced events for data, starting from tail */
		for (ssize_t i = buffer_event_nr - 1; i >= 0; --i) {
			struct buffer_event *be = buffer_events + i;
			size_t buf_i = be->buffer - buffers;

			/* write out buffer */
			if ((be->buffer->begin != be->buffer->end) &&
			    (be->wr_non_pollable ||
			     (be->wr_ready_out && !be->wr_have_errors))) {
				/* write out */
				ssize_t wr_bytes = write(be->buffer->wr_fd,
				                         be->buffer->buf + be->buffer->begin,
				                         be->buffer->end - be->buffer->begin);
				if (wr_bytes < 0) {
					log("[parent] Failed to write() %zu bytes: %m",
					     be->buffer->end - be->buffer->begin);
					be->wr_have_errors = true;
				}

				be->buffer->begin += wr_bytes;
			}

			/* read in buffer */
			if ((be->buffer->begin == be->buffer->end) &&
			    (be->rd_non_pollable ||
			     (be->rd_ready_in && !be->rd_have_errors))) {
				/* read in */
				ssize_t rd_bytes = read(be->buffer->rd_fd,
				                        be->buffer->buf,
				                        be->buffer->buf_size);
				if (rd_bytes < 0) {
					log("[parent] Failed to read() %zu bytes: %m",
					     be->buffer->buf_size);
					be->rd_have_errors = true;
				}
				if (rd_bytes == 0) {
					log("[parent] buffer_event %zu: buffer %zu input EOF",
					    i, buf_i);
					be->rd_ready_in = false;
					be->rd_hang_up = true;
				}

				be->buffer->begin = 0;
				be->buffer->end = rd_bytes;
			}

			if (be->rd_have_errors || be->wr_have_errors) {
				die_out("[parent] I/O for buffer %zu has errors",
				        buf_i);
			}
		}

		/* process coalesced events for closing, starting from head */
		for (size_t i = 0; i < buffer_event_nr; ++i) {
			struct buffer_event *be = buffer_events + i;
			size_t buf_i = be->buffer - buffers;

			/* close input if we're done with it */
			if (!be->rd_ready_in && be->rd_hang_up) {
				log("[parent] buffer_event %zu: buffer %zu input hangup, closing",
				    i, buf_i);

				if (!be->rd_non_pollable) {
					r = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, be->buffer->rd_fd, NULL);
					if (r < 0) {
						die_out("[parent] Failed to epoll_ctl(EPOLL_CTL_DEL) the exhausted input fd: %m");
					}
				}

				close(be->buffer->rd_fd);
				be->buffer->rd_fd = -1;
			}

			/* close output if we're done with it */
			if (be->buffer->rd_fd < 0 &&
			    (be->buffer->begin == be->buffer->end)) {
				log("[parent] buffer_event %zu: buffer %zu EOF, closing output",
				    i,
				    be->buffer - buffers);

				if (!be->wr_non_pollable) {
					r = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, be->buffer->wr_fd, NULL);
					if (r < 0) {
						die_out("[parent] Failed to epoll_ctl(EPOLL_CTL_DEL) the exhausted output fd: %m");
					}
				}

				close(be->buffer->wr_fd);
				be->buffer->wr_fd = -1;

				/* if we have just closed the output, exit */
				if (be->buffer == &buffers[child_nr]) {
					log("[parent] Done, waiting for children");
					for (;;) {
						r = waitpid(0, NULL, 0);
						if (r < 0) {
							if (errno == ECHILD) {
								break;
							}
							die_out("[parent] Failed to waitpid(0): %m");
						}
					}
					return 0;
				}
			}
		}
	}

out:
	kill_all_and_die(r);
}
