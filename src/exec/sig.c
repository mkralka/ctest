#include <errno.h>
#include <signal.h>
#include <string.h>

#include "sig.h"
#include "utils.h"

typedef struct saved_sigaction__ saved_sigaction_t__;
struct saved_sigaction__ {
	int f_saved;
	struct sigaction sigaction;

};

static const int signals__[] = {
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGABRT,
	SIGFPE,
#if 0
	SIGKILL,        /*  Can't be captured/stopped */
#endif
	SIGSEGV,
	SIGPIPE,
	SIGALRM,
	SIGTERM,
	SIGUSR1,
	SIGUSR2,
	SIGCHLD,
	SIGCONT,
#if 0
	SIGSTOP,        /*  Can't be captured/stopped */
#endif
	SIGTSTP,
	SIGTTIN,
	SIGTTOU,
 };

static saved_sigaction_t__ saved_sigaction__[countof(signals__)];
static void (*handler__)(int, void*);
static void *cookie__;

static void sighandler__(int signum, siginfo_t *unused(siginfo), void *unused(context))
{
	(*handler__)(signum, cookie__);
}

/**
 * Capture all signals that can be caught, invoking the specified handler.
 *
 * @param handler The function to invoke on signal capture; the first parameter
 *                is the signal number, the second is the provided cookie.
 * @param cookie  The cookie to pass to <code>handler</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
int sigcapture__(void (*handler)(int, void *), void *cookie)
{
	int result;
	int result_errno;
	size_t i;
	sigset_t sigset;
	sigset_t sigoldset;

	if (handler == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (handler__ != NULL) {
		errno = EBUSY;
		return -1;
	}

	/* Block signals so we can manipulate variables shared with the
	 * handlers. */
	sigfillset(&sigset);
	if (sigprocmask(SIG_BLOCK, &sigset, &sigoldset) != 0) {
		return -1;
	}

	for (i = 0; i < countof(signals__); ++i) {
		const int signum = signals__[i];
		saved_sigaction_t__ *const saved = saved_sigaction__ + i;
		struct sigaction *const old_sigact = saved->f_saved ? NULL : &saved->sigaction;
		struct sigaction sigact;
		int rc;

		memset(&sigact, 0, sizeof(sigact));
		sigact.sa_sigaction = &sighandler__;
		sigfillset(&sigact.sa_mask);
		sigact.sa_flags = SA_SIGINFO;

		if ((rc = sigaction(signum, &sigact, old_sigact)) == 0) {
			saved->f_saved = 1;
		} else if (errno != EINVAL) {
			/* Ignore invalid signal numbers -- EINVAL */
			result = rc;
			result_errno = errno;
			goto failure;
		}
	}

	handler__ = handler;
	cookie__ = cookie;

	sigprocmask(SIG_SETMASK, &sigoldset, NULL);
	errno = 0;
	return 0;

failure:
	sigprocmask(SIG_SETMASK, &sigoldset, NULL);

	/* An error occurred while attempting to register a signal handler,
	 * restore all registered handlers. */
	for (i = 0; i < countof(signals__); ++i) {
		const int signum = signals__[i];
		saved_sigaction_t__ *const saved = saved_sigaction__ + i;

		if (!saved->f_saved)
			continue;

		if (sigaction(signum, &saved->sigaction, NULL) == 0) {
			memset(saved, 0, sizeof(*saved));
		}
	}

	errno = result_errno;
	return result;
}

extern int sigrestore__(void)
{
	size_t i;
	int result = 0;
	int result_errno = 0;
	sigset_t sigset;
	sigset_t sigoldset;

	/* Block signals so we can manipulate variables shared with the
	 * handlers. */
	sigfillset(&sigset);
	if (sigprocmask(SIG_BLOCK, &sigset, &sigoldset) != 0) {
		return -1;
	}

	for (i = 0; i < countof(signals__); ++i) {
		const int signum = signals__[i];
		saved_sigaction_t__ *const saved = saved_sigaction__ + i;
		int rc;

		if (!saved->f_saved)
			continue;

		if ((rc = sigaction(signum, &saved->sigaction, NULL)) == 0) {
			memset(saved, 0, sizeof(*saved));
		} else {
			result = rc;
			result_errno = errno;
		}
	}

	if (result == 0) {
		handler__ = NULL;
		cookie__ = NULL;
	}

	sigprocmask(SIG_SETMASK, &sigoldset, NULL);
	errno = result_errno;
	return result;
}
