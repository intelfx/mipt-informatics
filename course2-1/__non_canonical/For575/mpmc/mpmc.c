/*
 * An implementation of the MPMC (multiple producer - multiple consumer) task
 * based on pthreads.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/*
 * Logging
 */

#define log(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define die(fmt, ...) do { log(fmt, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (0)

/*
 * Helpers
 */

unsigned long strtoul_or_die(const char *arg)
{
	char *endptr = NULL;
	errno = 0;
	unsigned long val = strtoul(arg, &endptr, 10);

	if (*arg == '\0' || *endptr != '\0') {
		die("Argument is not a valid non-negative decimal number: '%s'", arg);
	}

	if (errno) {
		die("Argument parse error: '%s': %m", arg);
	}

	return val;
}

void *malloc_or_die(size_t bytes)
{
	void *result = malloc(bytes);

	if (!result) {
		die("Failed to malloc() %zu bytes of memory: %m", bytes);
	}

	return result;
}

typedef unsigned long ulong;

/* the "document" structure, and a fixed-size queue of these implemented as a circular buffer */
struct document
{
	ulong prod_id;
	ulong doc_id;
} *queue, *queue_head, *queue_tail;
size_t queue_size, queue_used;
bool queue_going_away;

struct document *queue_get_and_advance(struct document **ptr)
{
	struct document *item = *ptr,
	                *next = item + 1;

	*ptr = next < queue + queue_size
	     ? next
	     : queue;

	return item;
}

/* the queue mutex -- serializes access to the queue */
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

/* the queue change condition variable -- used to notify other threads of
 * changing the @queue_used variable */
pthread_cond_t queue_cond_underflow = PTHREAD_COND_INITIALIZER,
               queue_cond_overflow = PTHREAD_COND_INITIALIZER;

/* a structure describing the parameters of a single producer */
struct producer_args
{
	ulong id;
	ulong amount, rate;
};

/* a structure describing the parameters of a single consumer */
struct consumer_args
{
	ulong id;
	ulong rate;
};

void *producer_main(void *arg)
{
	int r;

	struct producer_args *args = arg;

	long nsec_per_doc = 60 * (long)1e9 / args->rate;
	struct timespec time_per_doc = {
		.tv_sec = nsec_per_doc / (long)1e9,
		.tv_nsec = nsec_per_doc % (long)1e9
	};

	ulong doc_counter = 0;

	while (doc_counter < args->amount) {
		log("[producer %lu] waiting for %ld.%09ld s",
		    args->id, time_per_doc.tv_sec, time_per_doc.tv_nsec);

		r = nanosleep(&time_per_doc, NULL);
		if (r < 0) {
			if (errno == EINTR) {
				log("Producer interrupted, retrying");
				continue;
			}

			die("Failed to nanosleep() as a producer: %m");
		}

		struct document doc = {
			.prod_id = args->id,
			.doc_id = doc_counter++
		};

		log("[producer %lu] prepared a document %lu:%lu, locking queue",
		    args->id, doc.prod_id, doc.doc_id);

		r = pthread_mutex_lock(&queue_mutex);
		if (r < 0) {
			die("Failed to pthread_mutex_lock() the queue mutex: %m");
		}

		while (queue_used >= queue_size) {
			log("[producer %lu] queue full, waiting", args->id);

			r = pthread_cond_wait(&queue_cond_overflow, &queue_mutex);
			if (r < 0) {
				die("Failed to pthread_cond_wait() on the queue overflow cv: %m");
			}
		}

		log("[producer %lu] inserting the document %lu:%lu",
		    args->id, doc.prod_id, doc.doc_id);

		*queue_get_and_advance(&queue_tail) = doc;
		++queue_used;

		r = pthread_mutex_unlock(&queue_mutex);
		if (r < 0) {
			die("Failed to pthread_mutex_unlock() the queue mutex: %m");
		}

		r = pthread_cond_signal(&queue_cond_underflow);
		if (r < 0) {
			die("Failed to pthread_cond_signal() the queue underflow cv: %m");
		}
	}

	log("[producer %lu] done, exiting", args->id);
	return NULL;
}

void *consumer_main(void *arg)
{
	int r;

	struct consumer_args *args = arg;

	long nsec_per_doc = 60 * (long)1e9 / args->rate;
	struct timespec time_per_doc = {
		.tv_sec = nsec_per_doc / (long)1e9,
		.tv_nsec = nsec_per_doc % (long)1e9
	};

	for (;;) {
		log("[consumer %lu] locking queue",
		    args->id);

		r = pthread_mutex_lock(&queue_mutex);
		if (r < 0) {
			die("Failed to pthread_mutex_lock() the queue mutex: %m");
		}

		while (queue_used < 1) {
			if (queue_going_away) {
				log("[consumer %lu] queue empty and going away -- exiting", args->id);

				r = pthread_mutex_unlock(&queue_mutex);
				if (r < 0) {
					die("Failed to pthread_mutex_unlock() the queue mutex: %m");
				}

				return NULL;
			}

			log("[consumer %lu] queue empty, waiting", args->id);

			r = pthread_cond_wait(&queue_cond_underflow, &queue_mutex);
			if (r < 0) {
				die("Failed to pthread_cond_wait() on the queue underflow cv: %m");
			}
		}

		struct document doc = *queue_get_and_advance(&queue_head);
		--queue_used;

		log("[consumer %lu] retrieved the document %lu:%lu",
		    args->id, doc.prod_id, doc.doc_id);

		r = pthread_mutex_unlock(&queue_mutex);
		if (r < 0) {
			die("Failed to pthread_mutex_unlock() the queue mutex: %m");
		}

		r = pthread_cond_signal(&queue_cond_overflow);
		if (r < 0) {
			die("Failed to pthread_cond_signal() the queue overflow cv: %m");
		}

		log("[consumer %lu] waiting for %ld.%09ld s",
		    args->id, time_per_doc.tv_sec, time_per_doc.tv_nsec);

		r = nanosleep(&time_per_doc, NULL);
		if (r < 0) {
			if (errno == EINTR) {
				log("Producer interrupted, retrying");
				continue;
			}

			die("Failed to nanosleep() as a producer: %m");
		}

		printf("[consumer %lu] scanned the document %lu:%lu\n",
		       args->id, doc.prod_id, doc.doc_id);
	}
}

int main(int argc, char **argv)
{
	if (argc != 7) {
		die("This program requires six arguments.\n"
		    "Usage: %s <#producers> <production amount> <production rate 1/min> <queue length> <#consumers> <consumption rate 1/min>",
		    argv[0]);
	}

	int r;

	ulong prod_nr = strtoul_or_die(argv[1]),
	      prod_amount = strtoul_or_die(argv[2]),
	      prod_rate = strtoul_or_die(argv[3]),
	      cons_nr = strtoul_or_die(argv[5]),
	      cons_rate = strtoul_or_die(argv[6]);

	queue_size = strtoul_or_die(argv[4]);
	queue_used = 0;
	queue_going_away = false;

	queue = malloc_or_die(sizeof(struct document) * queue_size);
	queue_head = queue_tail = queue;

	pthread_t *producers = malloc_or_die(sizeof(pthread_t) * prod_nr),
	          *consumers = malloc_or_die(sizeof(pthread_t) * cons_nr);

	struct producer_args *producer_args = malloc_or_die(sizeof(struct producer_args) * prod_nr);
	struct consumer_args *consumer_args = malloc_or_die(sizeof(struct consumer_args) * cons_nr);

	/* spawn consumers */

	for (ulong i = 0; i < cons_nr; ++i) {
		struct consumer_args args = {
			.id = i,
			.rate = cons_rate
		};
		consumer_args[i] = args;

		r = pthread_create(&consumers[i], NULL, &consumer_main, &consumer_args[i]);
		if (r < 0) {
			die("Failed to pthread_create() a consumer thread: %m");
		}
	}

	/* spawn producers */
	for (ulong i = 0; i < prod_nr; ++i) {
		struct producer_args args = {
			.id = i,
			.amount = prod_amount,
			.rate = prod_rate
		};
		producer_args[i] = args;

		r = pthread_create(&producers[i], NULL, &producer_main, &producer_args[i]);
		if (r < 0) {
			die("Failed to pthread_create() a producer thread: %m");
		}
	}

	/* join producers */
	for (ulong i = 0; i < prod_nr; ++i) {
		r = pthread_join(producers[i], NULL);
		if (r < 0) {
			die("Failed to pthread_join() a producer thread: %m");
		}
	}

	log("Producers done");

	/* indicate termination for the consumer threads */
	r = pthread_mutex_lock(&queue_mutex);
	if (r < 0) {
		die("Failed to pthread_mutex_lock() the queue mutex to signal queue going away: %m");
	}

	queue_going_away = true;

	r = pthread_mutex_unlock(&queue_mutex);
	if (r < 0) {
		die("Failed to pthread_mutex_unlock() the queue mutex");
	}

	/* wake up all waiting consumers */
	r = pthread_cond_broadcast(&queue_cond_underflow);
	if (r < 0) {
		die("Failed to pthread_cond_broadcast() the queue underflow cv to wake up waiting consumers: %m");
	}

	/* join consumers */
	for (ulong i = 0; i < cons_nr; ++i) {
		r = pthread_join(consumers[i], NULL);
		if (r < 0) {
			die("Failed to pthread_join() a consumer thread: %m");
		}
	}

	log("Consumers done, exiting");

	return 0;
}
