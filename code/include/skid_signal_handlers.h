#ifndef __SKID_SIGNAL_HANDLERS__
#define __SKID_SIGNAL_HANDLERS__

#include <signal.h>     				// siginfo_t

// Function pointer to be used with struct sigaction's sa_handler member
typedef void (*SignalHandler)(int signum);
// Function pointer to be used with struct sigaction's sa_sigaction member
typedef void (*SignalHandlerExt)(int signum, siginfo_t *info, void *context);

/**************************************************************************************************/
/*************************** SIGNAL HANDLER ATOMIC VARIABLE DECLARATION ***************************/
/**************************************************************************************************/

// NOTES:
//	- Do *NOT* define these variables in your code.  They are defined in skid_signal_handlers.c.
//	- All flags are initialized to zero (0).
//	- Only use the assignment operator on these flags, or retrieve values, to ensure atomic usage.
//	- Signal handlers in this library indicate some/all/none flags utilized, per handler.

extern volatile sig_atomic_t skid_sig_hand_interrupted;  // Non-zero values indicate SIGINT handled
extern volatile sig_atomic_t skid_sig_hand_signum;       // Non-zero values indicate the signal num

/**************************************************************************************************/
/****************************** SA_HANDLER (SignalHandler) FUNCTIONS ******************************/
/**************************************************************************************************/

/*
 *	Description:
 *		This signal handler waits for all child processes to exit without hanging by calling:
 *			waitpid(-1, NULL, WNOHANG)
 *		This function conforms with SignalHandler type for use with struct sigaction.sa_handler.
 *		This function is reentrant and async-signal-safe.
 *
 *	Args:
 *		signum: The signal number.
 */
void handle_all_children(int signum);

/*
 *	Description:
 *		This handler sets the skid_sig_hand_interrupted atomic variable when SIGINT is handled.
 *		This function is reentrant and async-signal-safe.
 *
 *	Args:
 *		signum: The signal number.
 */
void handle_interruptions(int signum);

/*
 *	Description:
 *		This signal handler sets then skid_sig_hand_signum atomic variable when a signal is handled.
 *		This function is reentrant and async-signal-safe.
 *
 *	Args:
 *		signum: The signal number.
 */
void handle_signal_number(int signum);

/**************************************************************************************************/
/*************************** SA_SIGACTION (SignalHandlerExt) FUNCTIONS ****************************/
/**************************************************************************************************/

#endif  /* __SKID_SIGNAL_HANDLERS__ */
