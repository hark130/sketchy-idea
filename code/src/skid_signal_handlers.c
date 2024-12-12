/*
 *	This library defines functionality to help automate signal handling.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"				// SignalHandler
#include "skid_signals.h"				// SignalHandler
#include "skid_signal_handlers.h"		// Externed atomic variables
#include <errno.h>						// EINVAL
#include <sys/types.h>					// pid_t
#include <sys/wait.h>					// waitpid()

/**************************************************************************************************/
/*************************** SIGNAL HANDLER ATOMIC VARIABLE DEFINITION ****************************/
/**************************************************************************************************/

volatile sig_atomic_t skid_sig_hand_interrupted = 0;
volatile sig_atomic_t skid_sig_hand_signum = 0;

/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/

/****************************** SA_HANDLER (SignalHandler) FUNCTIONS ******************************/


void handle_all_children(int signum)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int errnum = errno;  // Perserve the errno value
	pid_t result = 0;    // Result of waitpid

	// HANDLE IT
	do
	{
		result = waitpid(-1, NULL, WNOHANG);  // Wait for any child process
		if (result > 0)
		{
			// The PID of a child process that exited; Keep going
		}
		else if (0 == result)
		{
			// All child processes have exited
			break;
		}
		else if (-1 == result)
		{
			// An error occurred
		}
	} while (result > 0);

	// CLEANUP
	errno = errnum;  // Restore the original errno value
}


void handle_interruptions(int signum)
{
	if (SIGINT == signum)
	{
		skid_sig_hand_interrupted = 1;  // Handled SIGINT
	}
}


void handle_signal_number(int signum)
{
	skid_sig_hand_signum = signum;  // The signal number handled
}


/*************************** SA_SIGACTION (SignalHandlerExt) FUNCTIONS ****************************/


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
