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
 *		Call sigprocmas() and handle errors in a standardized way.  Input is not validated.
 *
 *	Args:
 *		how: Controls the behavior of sigprocmask() (see: signprocmask(2)).
 *		set: Pointer to a new signal set.
 *		oldset: [Optional] Pointer to a storage location for the previous set.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int call_sigprocmask(int how, sigset_t *set, sigset_t *oldset);

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

/*
 *	Description:
 *		Empty this signal set and add signum to it.
 *
 *	Notes:
 *		Use sigemptyset() (see: sigsetops(3)) to initialize a signal set without adding a signal.
 *
 *	Args:
 *		set: The signal set to initialize.
 *		signum: The signal number to add.
 *
 *	Returns:
 *		On success, ENOERR is returned.  On error, errno is returned.
 */
int initialize_signal_set(sigset_t *set, int signum);


/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int block_signal(int signum, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value
	int how = SIG_BLOCK;  // Tells sigprocmask() to block signals
	sigset_t set;    	  // New signal set (intialized later)

	// BLOCK IT
	// Initialize the set
	result = initialize_signal_set(&set, signum);
	// Call sigprocmask
	if (ENOERR == result)
	{
		result = call_sigprocmask(how, &set, oldset);
	}

	// DONE
	return result;
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


int set_signal_handler_ext(int signum, SignalHandlerExt handler, int flags,
	                       struct sigaction *oldact)
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
		newact.sa_flags = flags | SA_SIGINFO;  // Explicitly adds the flag to use sa_sigaction
	}

	// SET IT
	if (ENOERR == result)
	{
		result = call_sigaction(signum, &newact, oldact);
	}

	// DONE
	return result;
}


int unblock_signal(int signum, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;    // Errno value
	int how = SIG_UNBLOCK;  // Tells sigprocmask() to unblock signals
	sigset_t set;    	    // New signal set (intialized later)

	// BLOCK IT
	// Initialize the set
	result = initialize_signal_set(&set, signum);
	// Call sigprocmask
	if (ENOERR == result)
	{
		result = call_sigprocmask(how, &set, oldset);
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

	// CALL IT
	if (sigaction(signum, newact, oldact))
	{
		result = errno;
		PRINT_ERROR(The call to sigaction() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int call_sigaddset(sigset_t *set, int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == set)
	{
		result = EINVAL;  // NULL pointer
	}

	// CALL IT
	if (ENOERR == result)
	{
		if(sigaddset(set, signum))
		{
			result = errno;
			PRINT_ERROR(The call to sigaddset() failed);
			PRINT_ERRNO(result);
		}
	}

	// DONE
	return result;
}


int call_sigemptyset(sigset_t *set)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// CALL IT
	if (sigemptyset(set))
	{
		result = errno;
		PRINT_ERROR(The call to sigaction() failed);
		PRINT_ERRNO(result);
	}

	// DONE
	return result;
}


int call_sigprocmask(int how, sigset_t *set, sigset_t *oldset)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// CALL IT
	if (sigprocmask(how, set, oldset))
	{
		result = errno;
		PRINT_ERROR(The call to sigprocmask() failed);
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

int initialize_signal_set(sigset_t *set, int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Errno value

	// INPUT VALIDATION
	if (NULL == set)
	{
		result = EINVAL;  // NULL pointer
	}

	// INITIALIZE IT
	// Empty it
	if (ENOERR == result)
	{
		result = call_sigemptyset(set);
	}
	// Add signum to the set
	if (ENOERR == result)
	{
		result = call_sigaddset(set, signum);
	}

	// DONE
	return result;
}
