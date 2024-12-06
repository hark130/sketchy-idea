#ifndef __SKID_SIGNALS__
#define __SKID_SIGNALS__

#include "skid_signal_handlers.h"		// SignalHandler, SignalHandlerExt
#include <signal.h>     				// Signal MACROs

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
 */
int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags, struct sigaction *oldact);

#endif  /* __SKID_SIGNALS__ */
