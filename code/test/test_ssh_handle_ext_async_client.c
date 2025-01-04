/*
 *	Manually test skid_signals.h's set_signal_handler_ext() and
 *	skid_signal_handlers.h's handle_ext_read_queue_int() functions.
 *
 *	This manual test code performs the following actions using handle_ext_read_queue_int:
 *	1. Registers itself with the server
 *	2. Reads character input from the user
 *	3. Sends each character, as an integer, to the server PID
 *	4. Uses the handle_ext_read_queue_int extended handler to wait for receipt from the server
 *	5. Clears the flag and sends the next character
 *	6. Repeat until the server stops responding
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_ext_async_client.bin <SERVER_PID> <SERVER_SIGNUM>

 *	If the client connects to the server, then start typing.
 */

// Standard includes
#include <errno.h>                  // EINVAL, EINTR, EIO
#include <stdbool.h>				// bool, false, true
#include <stdio.h>                  // fprintf()
#include <stdlib.h>					// exit()
#include <sys/wait.h>				// waitpid()
#include <unistd.h>					// fork()
// Local includes
#define SKID_DEBUG                  // The DEBUG output is doing double duty as test output
#include "skid_debug.h"             // DEBUG_INFO_STR, DEBUG_WARNG_STR, PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"			// ENOERR
#include "skid_memory.h"			// free_skid_string()
#include "skid_signal_handlers.h"	// handle_ext_read_queue_int()
#include "skid_signals.h"			// set_signal_handler_ext(), translate_signal_code()

#define MAX_REGISTER_ATTEMPTS 10  // Maximum number of attempts to register with the server
#define REGISTRATION_SLEEP 1      // Number of seconds to sleep betweeen registration attempts
#define REGISTRATION_VALUE '\n'   // The server doesn't care about the value but the server does

/*
 *	Queue a signal and data to this process by calling sigqueue() (see: sigqueue(3))
 */
int call_sigqueue(pid_t pid, int signum, int sival_int);

/*
 *	Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *	Call this when the signal handler indicates there's integer data to read.
 *	1. Verifies atomic variables (e.g., set, right signal number)
 *	2. Reads and validates expected value
 *	3. Resets atomic variables
 *
 *	Returns ENOERR for validated data, ENODATA for invalid data, or errno on failure.
 */
int process_queue(int signum, int value);

/*
 *	3. Resets atomic variables
 */
void process_queue_reset_flags(void);

/*
 *	1. Verifies atomic variables (e.g., set, right signal number)
 */
int process_queue_verify(int signum);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;       // Store errno and/or results here
	int flags = SA_RESTART;  // Modifies signal behavior: restart system calls across signals
	pid_t server_pid = 0;    // PID of the listening server
	int signum = SIGUSR1;    // Signal number to use for communication
	int user_input = 0;      // User input read from stdin

	// INPUT VALIDATION
	if (argc != 3)
	{
		print_usage(argv[0]);
		exit_code = EINVAL;
	}

	// SETUP
	// Server PID
	if (ENOERR == exit_code)
	{
		server_pid = atoi(argv[1]);
		if (server_pid < 1)
		{
			PRINT_ERROR(The call to atoi() failed to convert a valid PID);
			print_usage(argv[0]);
			exit_code = EINVAL;
		}
	}
	// Signal Number
	if (ENOERR == exit_code)
	{
		signum = atoi(argv[2]);
		if (signum < 1)
		{
			PRINT_ERROR(The call to atoi() failed to convert a valid signal number);
			print_usage(argv[0]);
			exit_code = EINVAL;
		}
	}
	// Setup Receive Signal Handler
	if (ENOERR == exit_code)
	{
		exit_code = set_signal_handler_ext(signum, handle_ext_read_queue_int, flags, NULL);
		if (exit_code)
		{
			PRINT_ERROR(The call to set_signal_handler_ext() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s %s is now handling signal %d: %s.\n", DEBUG_INFO_STR, argv[0],
				   signum, strsignal(signum));
		}
	}

	// RUN IT
	// Register With Server
	if (ENOERR == exit_code)
	{
		printf("%s %s is attempting to register with the server on PID %d using signal "
			   "number %d: %s\n", DEBUG_INFO_STR, argv[0], server_pid, signum, strsignal(signum));
		for (int i = 0; i < MAX_REGISTER_ATTEMPTS; i++)
		{
			putchar('.');  // Thinking...
			exit_code = call_sigqueue(server_pid, signum, REGISTRATION_VALUE);
			if (ENOERR == exit_code)
			{
				for (int j = 0; j < MAX_REGISTER_ATTEMPTS; j++)
				{
					// Process Data
					putchar('.');  // Thinking...
					exit_code = process_queue(signum, REGISTRATION_VALUE);
					if (ENOERR == exit_code)
					{
						printf("%s %s has successfully registered with the server\n",
							   DEBUG_INFO_STR, argv[0]);
						break;  // Success!
					}
					else if (ENODATA == exit_code)
					{
						// We're still waiting for the server's read receipt
						sleep(REGISTRATION_SLEEP);
					}
					else
					{
						break;  // This error is bad so stop looping
					}
				}
				// Validate process_queue() return value
				if (ENOERR == exit_code)
				{
					break;  // The server indicated that a successful send was read
				}
			}
		}
	}
	// Start Transmiting
	while (ENOERR == exit_code)
	{
		printf("%s Start typing data to the server...\n", DEBUG_INFO_STR);
		while(1)
		{
			// Get input
			user_input = fgetc(stdin);  // Read one character from the user
			if (EOF != user_input)
			{
				// printf("READ %c (%d)\n", user_input, user_input);  // DEBUGGING
				// Send it
				exit_code = call_sigqueue(server_pid, signum, user_input);
				if (ENOERR != exit_code)
				{
					PRINT_ERROR(The call to call_sigqueue() failed);
					PRINT_ERRNO(exit_code);
					break;  // Something went wrong
				}
				else
				{
					// PRINT_ERROR(Waiting on read receipt from the server);  // DEBUGGING
					// Verify receipt
					while(1)
					{
						if (ENOERR == process_queue(signum, user_input))
						{
							// PRINT_ERROR(GOT IT);  // DEBUGGING
							break;  // Receipt received and cleared
						}
					}
				}
			}
		}
	}

	// DONE
	exit(exit_code);
}


int call_sigqueue(pid_t pid, int signum, int sival_int)
{
	// LOCAL VARIABLES
	int result = ENOERR;      // Results of execution
	union sigval data;        // Data to send via sigqueue()

	// PREPARE
	data.sival_int = sival_int;

	// QUEUE IT
	if (sigqueue(pid, signum, data))
	{
		result = errno;
		PRINT_ERROR(The call to sigqueue() failed);
		if (ENOERR == result)
		{
			result = EINTR;  // Use this merely to indicate an error occurred
		}
		else
		{
			PRINT_ERRNO(result);
		}
		// FPRINTF_ERR("Attempted to sigqueue(%d, %d, %d)\n", pid, signum, sival_int);
	}

	// DONE
	return result;
}


void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s <SERVER_PID> <SERVER_SIGNUM>\n", prog_name);
}


int process_queue(int signum, int value)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution
	int data = 0;         // Integer data read from the atomic variable

	// PROCESS IT
	// 1. Verifies atomic variables (e.g., set, right signal number)
	result = process_queue_verify(signum);

	// 2. Reads and validates the value
	if (ENOERR == result)
	{
		data = skid_sig_hand_data_int;
		if (data != value)
		{
			result = ENODATA;			
		}
	}

	// Reset the atomic variables if a value was read
	if (ENOERR == result)
	{
		process_queue_reset_flags();
	}

	// DONE
	return result;
}


void process_queue_reset_flags(void)
{
	skid_sig_hand_queue = 0;
	skid_sig_hand_data_int = 0;
	skid_sig_hand_pid = 0;
	skid_sig_hand_sigcode = 0;
	skid_sig_hand_signum = 0;
	skid_sig_hand_uid = 0;
}


int process_queue_verify(int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;            // Results of execution
	QueueData_t integer = Integer;  // Only accepting integer values in the queue

	// VERIFY VARIABLES
	if (integer != skid_sig_hand_queue)
	{
		result = ENODATA;  // Nothing that we care about
	}
	else if (signum != skid_sig_hand_signum)
	{
		result = ENODATA;  // Wrong signal number
	}

	// DONE
	return result;
}
