/*
 *	Manually test skid_signals.h's set_signal_handler_ext() and
 *	skid_signal_handlers.h's handle_ext_signal_code() functions.
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_ext_sending_process.bin

* ...then...

kill -s <SIGNAL_NUMBER> `pidof ./code/dist/test_ssh_handle_ext_sending_process.bin`

* -or-

kill -<SIGNAL_NAME> `pidof ./code/dist/test_ssh_handle_ext_sending_process.bin`

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
#include "devops_code.h"			// call_sigqueue()
#include "skid_debug.h"             // DEBUG_INFO_STR, DEBUG_WARNG_STR, PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"			// ENOERR
#include "skid_memory.h"			// free_skid_string()
#include "skid_signal_handlers.h"	// handle_ext_sending_process()
#include "skid_signals.h"			// set_signal_handler_ext(), translate_signal_code()


#define SLEEP_TIME 60  // Number of seconds to sleep while waiting to be interrupted


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
				exit_code = set_signal_handler_ext(signum, handle_ext_sending_process, flags, NULL);
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
		printf("%s - Interrupt this process by sending any signal to PID %d using or else this "
			   "process will self-terminate using sigqueue().\n", DEBUG_INFO_STR,
			   getpid());
		while (0 == skid_sig_hand_ext)
		{
			printf("%s - Waiting for a signal [%02d seconds left]...\n", DEBUG_INFO_STR, countdown);
			sleep(1);
			countdown--;
			if (countdown <= 0)
			{
				call_sigqueue(getpid(), SIGUSR1, (int)'?');  // BOOM?
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
		printf("\n%s - Handled signal %d: %s.\n", DEBUG_INFO_STR, skid_sig_hand_signum,
			   strsignal(skid_sig_hand_signum));
		printf("%s - Signal code %d: %s.\n", DEBUG_INFO_STR, skid_sig_hand_sigcode, sig_code_str);
		printf("%s - Sending PID: %d\n", DEBUG_INFO_STR, skid_sig_hand_pid);
		printf("%s - Sending UID: %d\n", DEBUG_INFO_STR, skid_sig_hand_uid);
	}

	// CLEANUP
	if (NULL != sig_code_str)
	{
		free_skid_string(&sig_code_str);  // Best effort
	}

	// DONE
	exit(exit_code);
}
