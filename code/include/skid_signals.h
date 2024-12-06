#ifndef __SKID_SIGNALS__
#define __SKID_SIGNALS__

#include "skid_signal_handlers.h"		// SignalHandler, SignalHandlerExt
#include <signal.h>     				// Signal MACROs

/*
 *	Description:
 *		Block a signal.  The result will be the union of the current set and signum.
 *		It is not possible to block SIGKILL or SIGSTOP.  Attempts to do so are silently ignored.
 *
 *	Args:
 *		signum: The signal number.
 *		oldset: [Optional] If non-NULL, the previous set will be saved here.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int block_signal(int signum, sigset_t *oldset);

/*
 *	Description:
 *		Establish handler as the signum signal handler.
 *
 *	Args:
 *		signum: The signal number.
 *		handler: Function pointer of the signal handler.  Passed to sigaction() by way of
 *			struct sigaction.sa_handler.
 *		flags: Passed to sigaction() by way of struct sigaction.sa_flags.
 *		oldact: [Optional] If non-NULL, the previous action is saved here.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int set_signal_handler(int signum, SignalHandler handler, int flags, struct sigaction *oldact);

/*
 *	Description:
 *		Establish handler as the signum signal handler.
 *
 *	Args:
 *		signum: The signal number.
 *		handler: Function pointer of the extended signal handler.  Passed to sigaction() by way of
 *			struct sigaction.sa_sigaction.
 *		flags: Passed to sigaction() by way of struct sigaction.sa_flags.
 *		oldact: [Optional] If non-NULL, the previous action is saved here.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags,
	                       struct sigaction *oldact);

/*
 *	Description:
 *		Unblock a signal.  The signal number will be removed from the current set of blocked
 *		signals.  It is permissible to attempt to unblock a signal which is not blocked.
 *
 *	Args:
 *		signum: The signal number.
 *		oldset: [Optional] If non-NULL, the previous set will be saved here.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int unblock_signal(int signum, sigset_t *oldset);

#endif  /* __SKID_SIGNALS__ */
