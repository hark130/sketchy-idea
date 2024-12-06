#ifndef __SKID_SIGNAL_HANDLERS__
#define __SKID_SIGNAL_HANDLERS__

#include <signal.h>     				// siginfo_t

// Function pointer to be used with struct sigaction's sa_handler member
typedef void (*SignalHandler)(int signum);
// Function pointer to be used with struct sigaction's sa_sigaction member
typedef void (*SignalHandlerExt)(int signum, siginfo_t *info, void *context);

/**************************************************************************************************/
/****************************** SA_HANDLER (SignalHandler) FUNCTIONS ******************************/
/**************************************************************************************************/

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

/**************************************************************************************************/
/*************************** SA_SIGACTION (SignalHandlerExt) FUNCTIONS ****************************/
/**************************************************************************************************/

#endif  /* __SKID_SIGNAL_HANDLERS__ */
