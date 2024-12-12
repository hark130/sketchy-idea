/*
 *	Manually test skid_signals.h's block_signal() and unblock_signal() functions.
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_interruptions.bin

* ...then...

kill -s 2 `pidof ./code/dist/test_ssh_handle_interruptions.bin`

* -or-

kill -SIGINT `pidof ./code/dist/test_ssh_handle_interruptions.bin`

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
#include "skid_signal_handlers.h"	// handle_interruptions()
#include "skid_signals.h"			// set_signal_handler()


#define SLEEP_TIME 5  // Number of seconds to sleep while waiting to be interrupted


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;       // Store errno and/or results here
	int signum = SIGINT;     // Signal number to use
	int flags = SA_RESTART;  // Modifies signal behavior: restart system calls across signals

	// INPUT VALIDATION
	if (argc != 1)
	{
	   fprintf(stderr, "Usage: %s\n", argv[0]);
	   exit_code = EINVAL;
	}

	// RUN IT
	// Handle a signal
	if (!exit_code)
	{
		printf("%s - Handling signal %d: %s\n", DEBUG_INFO_STR, signum, strsignal(signum));
		exit_code = set_signal_handler(signum, handle_interruptions, flags, NULL);
		if (exit_code)
		{
			PRINT_ERROR(The call to set_signal_handler() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s - %s is now handling signal %d.\n", DEBUG_INFO_STR, argv[0], signum);
		}
	}
	// Process until interrupted
	if (!exit_code)
	{
		printf("%s - Interrupt this process by sending signal %d to PID %d.\n", DEBUG_INFO_STR,
			   signum, getpid());
		while (0 == skid_sig_hand_interrupted)
		{
			printf("%s - Waiting for an interruption...\n", DEBUG_INFO_STR);
			sleep(SLEEP_TIME);
		}
	}
	// Process was interrupted
	if (!exit_code)
	{
		// No other way to get here...
		printf("\n%s - Interrupted by signal %d.\n", DEBUG_INFO_STR, signum);
	}

	// DONE
	exit(exit_code);
}
