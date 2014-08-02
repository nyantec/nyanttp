/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ev.h>

#include <nyanttp/ny.h>
#include <nyanttp/expect.h>

unsigned int ny_version_major() {
	return NY_VERSION_MAJOR;
}

unsigned int ny_version_minor() {
	return NY_VERSION_MINOR;
}

unsigned int ny_version_micro() {
	return NY_VERSION_MICRO;
}

/**
 * \brief Real context initialisation
 *
 * \param[out] ny Pointer to context structure
 *
 * \return Zero on success or non-zero on error
 */
int ny_init_(struct ny *restrict ny) {
	assert(ny);

	int ret = -1;

	/* Check libev version */
	if (unlikely(ev_version_major() != EV_VERSION_MAJOR
		|| ev_version_minor() < EV_VERSION_MINOR)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVVER);
		goto exit;
	}

	/* Determine system page size */
	int page_size = sysconf(_SC_PAGE_SIZE);
	if (unlikely(page_size < 0)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto exit;
	}

	ny->page_size = page_size;

	/* Create new event loop */
	ny->loop = ev_loop_new(EVFLAG_AUTO);
	if (unlikely(!ny->loop)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVINIT);
		goto exit;
	}

	ret = 0;

exit:
	return ret;
}

void ny_destroy(struct ny *restrict ny) {
	assert(ny);

	assert(ny->loop);
	ev_loop_destroy(ny->loop);
	ny->loop = NULL;
}

static void child_event(struct ev_loop *loop, struct ev_child *child,
	int revents) {
	assert(loop);
	assert(child);
	assert(revents & EV_CHILD);

	/* TODO: Do something useful */
	ev_break(loop, EVBREAK_ALL);
}

int ny_run(struct ny *restrict ny, unsigned nproc) {
	assert(ny);
	assert(nproc < sysconf(_SC_CHILD_MAX));

	int ret = -1;

#if NY_MPROC
	struct ev_child procv[nproc];
	memset(procv, 0, sizeof procv);

	/* Initialise default event loop */
	struct ev_loop *loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!loop)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVINIT);
		goto exit;
	}

	/* Fork children */
	for (unsigned iter = 0; iter < nproc; ++iter) {
		pid_t proc = fork();
		if (unlikely(proc < 0)) {
			ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
			goto reap;
		}
		else if (proc == 0) {
			/* Run event loop in the child */
			ev_loop_fork(ny->loop);
			ev_run(ny->loop, 0);
			exit(EXIT_SUCCESS);
		}

		/* Setup child watcher */
		ev_child_init(procv + iter, child_event, proc, 0);
		procv[iter].data = procv + iter;
		ev_child_start(loop, procv + iter);
	}

	ev_run(loop, 0);
#else
	ev_run(ny->loop, 0);
#endif

	ret = 0;

#if NY_MPROC
reap:
	/* Stop watchers and kill children */
	for (unsigned iter = 0; iter < nproc; ++iter) {
		if (likely(procv[iter].pid)) {
			ev_child_stop(loop, procv + iter);
			kill(procv[iter].pid, SIGTERM);
		}
	}
#endif

exit:
	return ret;
}
