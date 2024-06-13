/*
 *	This library defines functionality to help automate signal handling.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO(), PRINT_ERROR()

#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


void handle_all_children(int signum);
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


int set_signal_handler(int signum, SignalHandler handler, int flags, struct sigaction *oldact)
{
	// LOCAL VARIABLES
	int result = ENOERR;      // Errno value
	struct sigaction newact;  // Argument for sigaction()

	// INPUT VALIDATION
	if (NULL == handler)
	{
		result = EINVAL;  // NULL pointer
	}

	// SETUP
	// Initialize the struct
	if (ENOERR == result)
	{
		// Clear the struct
		memset(&newact, 0x0, sizeof(newact));
		// Exclude all the signals from the set
		if (sigemptyset(&(newact.sa_mask)))
		{
			result = errno;
			PRINT_ERROR(The call to sigemptyset() failed);
			PRINT_ERRNO(result);
		}
	}
	// Assign the struct values
	if (ENOERR == result)
	{
		newact.sa_handler = handler;
		newact.sa_flags = flags;
	}

	// SET IT
	if (ENOERR == result)
	{
		if (sigaction(signum, &newact, oldact))
		{
			result = errno;
			PRINT_ERROR(The call to sigaction() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/
