/*
 *	This library defines functionality to help automate signal handling.
 */

#define SKID_DEBUG						// Enable DEBUG logging

#include "skid_debug.h"				  	// PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"				// NOERR
#include "skid_signals.h"				// Signal MACROs, SignalHandler
#include <errno.h>						// EINVAL
#include <sys/types.h>					// pid_t
#include <sys/wait.h>					// waitpid()


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Call sigaction() and handle errors in a standardized way.  Input is not validated.
 *
 *	Args:
 *		signum: The signal number.
 *		newact: Pointer to a new sigaction struct.
 *		oldact: [Optional] Pointer to a storage location for the old sigaction struct.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int call_sigaction(int signum, struct sigaction *newact, struct sigaction *oldact);

/*
 *	Description:
 *		Initialize action's memory and empty the signal set.
 *
 *	Args:
 *		action: The signal set to initialize.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int initialize_sigaction_struct(struct sigaction *action);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


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
		result = initialize_sigaction_struct(&newact);
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
		result = call_sigaction(signum, &newact, oldact);
	}

	// DONE
	return result;
}


int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags, struct sigaction *oldact)
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
		result = initialize_sigaction_struct(&newact);
	}
	// Assign the struct values
	if (ENOERR == result)
	{
		newact.sa_sigaction = handler;
		newact.sa_flags = flags;
	}

	// SET IT
	if (ENOERR == result)
	{
		result = call_sigaction(signum, &newact, oldact);
	}

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/

int call_sigaction(int signum, struct sigaction *newact, struct sigaction *oldact)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// SET IT
	if (sigaction(signum, newact, oldact))
	{
		result = errno;
		PRINT_ERROR(The call to sigaction() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}

int initialize_sigaction_struct(struct sigaction *action)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == action)
	{
		result = EINVAL;  // NULL pointer
	}

	// INITIALIZE IT
	if (ENOERR == result)
	{
		// Clear the struct
		memset(action, 0x0, sizeof(struct sigaction));
		// Exclude all the signals from the set
		if (sigemptyset(&(action->sa_mask)))
		{
			result = errno;
			PRINT_ERROR(The call to sigemptyset() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}
