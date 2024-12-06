#ifndef __SKID_SIGNALS__
#define __SKID_SIGNALS__

#include <signal.h>     // Signal MACROs

// Function pointer to be used with struct sigaction's sa_handler member
typedef void (*SignalHandler)(int signum);

/*
 *	Description:
 *		This signal handler waits for all child processes to exit without hanging by calling:
 *			waitpid(-1, NULL, WNOHANG)
 *		This function conforms with SignalHandler type for use with struct sigaction.sa_handler.
 *
 *	Args:
 *		signum: The signal number.
 */
void handle_all_children(int signum);

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

#endif  /* __SKID_SIGNALS__ */
