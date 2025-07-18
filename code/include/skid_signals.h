#ifndef __SKID_SIGNALS__
#define __SKID_SIGNALS__

#include <signal.h>                         // Signal MACROs
#include "skid_signal_handlers.h"           // SignalHandler, SignalHandlerExt

/*
 *  Description:
 *      Block a signal.  The result will be the union of the current set and signum.
 *      It is not possible to block SIGKILL or SIGSTOP.  Attempts to do so are silently ignored.
 *
 *  Args:
 *      signum: The signal number.
 *      oldset: [Optional] If non-NULL, the previous set will be saved here.
 *
 *  Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int block_signal(int signum, sigset_t *oldset);

/*
 *  Description:
 *      Establish handler as the signum signal handler.
 *
 *  Args:
 *      signum: The signal number.
 *      handler: Function pointer of the signal handler.  Passed to sigaction() by way of
 *          struct sigaction.sa_handler.
 *      flags: Passed to sigaction() by way of struct sigaction.sa_flags.
 *      oldact: [Optional] If non-NULL, the previous action is saved here.
 *
 *  Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int set_signal_handler(int signum, SignalHandler handler, int flags, struct sigaction *oldact);

/*
 *  Description:
 *      Establish handler as the signum signal handler.
 *
 *  Args:
 *      signum: The signal number.
 *      handler: Function pointer of the extended signal handler.  Passed to sigaction() by way of
 *          struct sigaction.sa_sigaction.
 *      flags: Passed to sigaction() by way of struct sigaction.sa_flags.
 *      oldact: [Optional] If non-NULL, the previous action is saved here.
 *
 *  Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags,
                           struct sigaction *oldact);

/*
 *    Description:
 *      Translate a signal number's signal code into a human-readable description.  This function
 *      replicates the behavior of strsignal() but for signal codes (see: sigaction(2)'s
 *      description of the siginfo_t data type's si_code field).
 *      It is the caller's responsibility to free the return value using skid_memory's
 *      free_skid_string().
 *
 *    Args:
 *      signum: The signal number.
 *      sigcode: The signal code.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *    Returns:
 *      Heap-allocated string describing the signal code with relation to the signal number.
 *      NULL on error (check errnum for details).
 */
char *translate_signal_code(int signum, int sigcode, int *errnum);

/*
 *    Description:
 *      Unblock a signal.  The signal number will be removed from the current set of blocked
 *      signals.  It is permissible to attempt to unblock a signal which is not blocked.
 *
 *    Args:
 *      signum: The signal number.
 *      oldset: [Optional] If non-NULL, the previous set will be saved here.
 *
 *    Returns:
 *      On success, ENOERR is returned.  On error, errno is returned.
 */
int unblock_signal(int signum, sigset_t *oldset);

#endif  /* __SKID_SIGNALS__ */
