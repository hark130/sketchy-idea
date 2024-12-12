/*
 *	Manually test skid_signals.h's set_signal_handler_ext() and
 *	skid_signal_handlers.h's handle_ext_signal_code() functions.
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_ext_signal_code.bin

* ...then...

kill -s <SIGNAL_NUMBER> `pidof ./code/dist/test_ssh_handle_ext_signal_code.bin`

* -or-

kill -<SIGNAL_NAME> `pidof ./code/dist/test_ssh_handle_ext_signal_code.bin`

 *
 */

// Standard includes
#include <errno.h>                  // EINVAL, EINTR
#include <signal.h>					// raise()
#include <stdio.h>                  // fprintf()
#include <stdlib.h>					// exit()
#include <unistd.h>					// sleep()
// Local includes
#define SKID_DEBUG                  // The DEBUG output is doing double duty as test output
#include "skid_debug.h"             // DEBUG_INFO_STR, DEBUG_WARNG_STR, PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"			// ENOERR
#include "skid_memory.h"			// free_skid_string()
#include "skid_signal_handlers.h"	// handle_ext_signal_code()
#include "skid_signals.h"			// set_signal_handler_ext(), translate_signal_code()


#define SLEEP_TIME 60  // Number of seconds to sleep while waiting to be interrupted

/*
 *	Raise a signal number by calling raise() (see: raise(3))
 */
int raise_signal(int signum);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;           // Store errno and/or results here
	int flags = SA_RESTART;      // Modifies signal behavior: restart system calls across signals
	char *sig_code_str = NULL;   // Heap-allocated translation of a received signal code
	int countdown = SLEEP_TIME;  // It's the final countdown!
 
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
				exit_code = set_signal_handler_ext(signum, handle_ext_signal_code, flags, NULL);
				if (exit_code)
				{
					PRINT_ERROR(The call to set_signal_handler_ext() failed);
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
		printf("%s - Interrupt this process by sending any signal to PID %d or else this process "
			   "will self-terminate by raising a SIGILL signal.\n", DEBUG_INFO_STR,
			   getpid());
		while (0 == skid_sig_hand_ext)
		{
			printf("%s - Waiting for a signal [%02d seconds left]...\n", DEBUG_INFO_STR, countdown);
			sleep(1);
			countdown--;
			if (countdown <= 0)
			{
				raise_signal(SIGILL);  // BOOM!
			}
		}
	}
	// Process was interrupted
	// Translate the signal code
	if (!exit_code)
	{
		sig_code_str = translate_signal_code(skid_sig_hand_signum, skid_sig_hand_sigcode,
			                                 &exit_code);
		if (exit_code)
		{
			PRINT_ERROR(The call to translate_signal_code() failed);
			PRINT_ERRNO(exit_code);
		}
	}
	// Print signal handler results
	if (!exit_code)
	{
		printf("\n%s - Interrupted by signal %d: %s.\n", DEBUG_INFO_STR, skid_sig_hand_signum,
			   strsignal(skid_sig_hand_signum));
		printf("%s - Signal code %d: %s.\n", DEBUG_INFO_STR, skid_sig_hand_sigcode, sig_code_str);
	}

	// CLEANUP
	if (NULL != sig_code_str)
	{
		free_skid_string(&sig_code_str);  // Best effort
	}

	// DONE
	exit(exit_code);
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
