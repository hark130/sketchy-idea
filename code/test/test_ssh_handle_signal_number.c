/*
 *	Manually test skid_signals.h's set_signal_handler() and
 *	skid_signal_handlers.h's handle_signal_number() functions.
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_signal_number.bin

* ...then...

kill -s <SIGNAL_NUMBER> `pidof ./code/dist/test_ssh_handle_signal_number.bin`

* -or-

kill -<SIGNAL_NAME> `pidof ./code/dist/test_ssh_handle_signal_number.bin`

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
#include "skid_signal_handlers.h"	// handle_signal_number()
#include "skid_signals.h"			// set_signal_handler()


#define SLEEP_TIME 5  // Number of seconds to sleep while waiting to be interrupted


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;       // Store errno and/or results here
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
		for (int signum = 1; signum <= 64; signum++)
		{
			if (SIGKILL == signum || SIGSTOP == signum)
			{
				printf("%s - Skipping signal %d: %s\n", DEBUG_WARNG_STR, signum, strsignal(signum));
			}
			else if (32 == signum || 33 == signum)
			{
				printf("%s - Skipping signal %d: %s\n", DEBUG_WARNG_STR, signum, strsignal(signum));
			}
			else
			{
				exit_code = set_signal_handler(signum, handle_signal_number, flags, NULL);
				if (exit_code)
				{
					PRINT_ERROR(The call to set_signal_handler() failed);
					PRINT_ERRNO(exit_code);
					break;  // Let's stop here
				}
				else
				{
					printf("%s - %s is now handling signal %d: %s.\n", DEBUG_INFO_STR, argv[0],
						   signum, strsignal(signum));
				}
			}
		}

	}
	// Process until interrupted
	if (!exit_code)
	{
		printf("%s - Interrupt this process by sending any signal to PID %d.\n", DEBUG_INFO_STR,
			   getpid());
		while (0 == skid_sig_hand_signum)
		{
			printf("%s - Waiting for a signal...\n", DEBUG_INFO_STR);
			sleep(SLEEP_TIME);
		}
	}
	// Process was interrupted
	if (!exit_code)
	{
		printf("\n%s - Interrupted by signal %d: %s.\n", DEBUG_INFO_STR, skid_sig_hand_signum,
			   strsignal(skid_sig_hand_signum));
	}

	// DONE
	exit(exit_code);
}
