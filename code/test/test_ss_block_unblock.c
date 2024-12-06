/*
 *	Manually test skid_signals.h's block_signal() and unblock_signal() functions.
 *
 *	Copy/paste the following...

./code/dist/test_ss_block_unblock.bin

 *
 */

// Standard includes
#include <errno.h>                  // EINVAL, EINTR
#include <signal.h>					// raise()
#include <stdbool.h>				// bool, false, true
#include <stdio.h>                  // fprintf()
#include <stdlib.h>					// exit()
#include <unistd.h>					// sleep()
// Local includes
#define SKID_DEBUG                  // The DEBUG output is doing double duty as test output
#include "skid_debug.h"             // DEBUG_INFO_STR, DEBUG_WARNG_STR, PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"			// ENOERR
#include "skid_signals.h"			// block_signal(), unblock_signal()


/*
 *	Check if signum is pending for delivery to the calling thread.
 */
bool is_signal_pending(int signum, int *errnum);


/*
 *	Raise a signal number by calling raise() (see: raise(3))
 */
int raise_signal(int signum);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;     // Store errno and/or results here
	int signum = SIGUSR1;  // Signal number to use

	// INPUT VALIDATION
	if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s\n", argv[0]);
	   exit_code = EINVAL;
	}

	// RUN IT
	// Block a signal
	if (!exit_code)
	{
		fprintf(stdout, "%s - Blocking signal %d: %s\n", DEBUG_INFO_STR, signum, strsignal(signum));
		exit_code = block_signal(signum, NULL);
		if (exit_code)
		{
			PRINT_ERROR(The call to block_signal() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s - %s is now blocking signal %d.\n", DEBUG_INFO_STR, argv[0], signum);
		}
	}
	// Raise the signal
	if (!exit_code)
	{
		fprintf(stdout, "%s - About to raise a signal %d: %s; This process should survive.\n",
			    DEBUG_INFO_STR, signum, strsignal(signum));
		exit_code = raise_signal(signum);
		fprintf(stdout, "%s - This process survived signal %d.\n", DEBUG_INFO_STR, signum);
	}
	// Prepare to unblock the signal
	if (!exit_code)
	{
		if (true == is_signal_pending(signum, &exit_code))
		{
			fprintf(stdout, "%s - Signal %d is still pending; This process will not survive "
				    "the unblocking.\n", DEBUG_WARNG_STR, signum);
		}
	}
	// Unblock the signal
	if (!exit_code)
	{
		fprintf(stdout, "%s - Unblocking signal %d: %s\n", DEBUG_INFO_STR, signum,
			    strsignal(signum));
		exit_code = unblock_signal(signum, NULL);
		if (exit_code)
		{
			PRINT_ERROR(The call to unblock_signal() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s is no longer blocking signal %d.\n", argv[0], signum);
		}
	}
	// Raise the signal again
	if (!exit_code)
	{
		fprintf(stdout, "%s - About to raise a signal %d: %s; This process should not survive.\n",
			    DEBUG_INFO_STR, signum, strsignal(signum));
		exit_code = raise_signal(signum);
		PRINT_ERROR(If you see this then know that the manual test has failed);
	}

	// DONE
	exit(exit_code);
}


bool is_signal_pending(int signum, int *errnum)
{
	// LOCAL VARIABLES
	int result = ENOERR;      // Store errno values here
	int retval = 0;           // Return value of sigismember()
	bool is_pending = false;  // Prove this wrong
	sigset_t pending;         // Pending signals

	// IS IT?
	// Get the pending signal set
    if(sigpending(&pending))
    {
    	result = errno;
		PRINT_ERROR(The call to sigpending() failed);
		PRINT_ERRNO(result);
    }
    // Is our signal in there?
    if (ENOERR == result)
    {
    	retval = sigismember(&pending, signum);
    	if (1 == retval)
    	{
    		is_pending = true;
    	}
    	else if (-1 == retval)
    	{
    		result = errno;
			PRINT_ERROR(The call to sigismember() failed);
			PRINT_ERRNO(result);
    	}
    }

	// DONE
	if (errnum)
	{
		*errnum = result;
	}
	return is_pending;
}


int raise_signal(int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;

	// RAISE IT
	if (raise(signum))
	{
		result = errno;
		PRINT_ERROR(The call to raise() failed);
		if (ENOERR == result)
		{
			result = EINTR;  // Use this merely to indicate an error occurred
		}
		else
		{
			PRINT_ERRNO(result);
		}
		FPRINTF_ERR("Attempted to raise(%d)\n", signum);
	}

	// DONE
	return result;
}
