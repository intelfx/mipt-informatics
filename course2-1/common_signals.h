#pragma once

#include "common_util.h"

/*
 * Operations with signals
 */

static inline int sigaction_many(struct sigaction *act, ...)
{
	int r, sig;
	int has_errors = 0;

	assert(act);

	va_list ap;

	va_start(ap, act);
	while ((sig = va_arg(ap, int)) > 0) {
		sigaddset(&act->sa_mask, sig);
	}
	va_end(ap);

	va_start(ap, act);
	while ((sig = va_arg(ap, int)) > 0) {
		r = sigaction(sig, act, NULL);
		if (r < 0) {
			log("Failed to call sigaction() to install handler for signal %d: %m", sig);
			has_errors = -1;
		}
	}
	va_end(ap);

	return has_errors;
}

/*
 * Constructs a sigset_t with given signals included in it.
 * Signal 0 marks the end of the argument list.
 */
static inline sigset_t sigset_many(int signal, ...)
{
	sigset_t result;
	va_list ap;

	assert(signal != 0);

	sigemptyset(&result);
	sigaddset(&result, signal);

	va_start(ap, signal);
	while ((signal = va_arg(ap, int)) > 0) {
		sigaddset(&result, signal);
	}
	va_end(ap);

	return result;
}

/*
 * Constructs a sigset_t with all except given signals included in it.
 * Signal 0 marks the end of the argument list.
 */
static inline sigset_t sigset_inverse_many(int signal, ...)
{
	sigset_t result;
	va_list ap;

	assert(signal != 0);

	sigemptyset(&result);
	sigfillset(&result);
	sigdelset(&result, signal);

	va_start(ap, signal);
	while ((signal = va_arg(ap, int)) > 0) {
		sigdelset(&result, signal);
	}
	va_end(ap);

	return result;
}

