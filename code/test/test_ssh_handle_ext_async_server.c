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

./code/dist/test_ssh_handle_ext_async_server.bin <SERVER_SIGNUM>

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
#include "devops_code.h"			// call_sigqueue()
#include "skid_debug.h"             // DEBUG_INFO_STR, DEBUG_WARNG_STR, PRINT_ERRNO(), PRINT_ERROR()
#include "skid_macros.h"			// ENOERR
#include "skid_memory.h"			// free_skid_string()
#include "skid_signal_handlers.h"	// handle_ext_read_queue_int()
#include "skid_signals.h"			// set_signal_handler_ext(), translate_signal_code()

#define REGISTRATION_SLEEP 1      // Number of seconds to sleep betweeen registration attempts

/*
 *	Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *	Call this when the signal handler indicates there's integer data to read.
 *	1. Verifies atomic variables (e.g., set, right signal number)
 *	2. Reads and prints values from src_pid
 *	3. Resets atomic variables
 *	4. Sends the read receipt
 *
 *	Returns ENOERR on success, errno on failure.
 */
int process_queue(int signum, pid_t src_pid);

/*
 *	2. Reads and prints value
 */
int process_queue_read(void);

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
	pid_t client_pid = 0;    // PID of the listening server
	int signum = SIGUSR1;    // Signal number to use for communication
	int client_input = 0;    // Value read from the incoming signal

	// INPUT VALIDATION
	if (argc != 2)
	{
		print_usage(argv[0]);
		exit_code = EINVAL;
	}

	// SETUP
	// Signal Number
	if (ENOERR == exit_code)
	{
		signum = atoi(argv[1]);
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
	// Prompt the user to register a client
	if (ENOERR == exit_code)
	{
		printf("%s %s is waiting for the first client to register with the server on PID %d "
			   "using signal number %d: %s\n", DEBUG_INFO_STR, argv[0], getpid(), signum,
			   strsignal(signum));
		while (1)
		{
			fprintf(stderr, ".");  // Waiting...
			if (ENOERR == process_queue_verify(signum))
			{
				client_pid = skid_sig_hand_pid;  // This is the client's PID
				client_input = skid_sig_hand_data_int;  // The value sent by the client
				process_queue_reset_flags();  // Reset the flags
				exit_code = call_sigqueue(client_pid, signum, client_input);  // Send the receipt
				if (ENOERR != exit_code)
				{
					PRINT_ERROR(The read receipt call to call_sigqueue() failed);
					PRINT_ERRNO(exit_code);
					break;
				}
				else
				{
					printf("\n%s Registered PID %d as a client\n", DEBUG_INFO_STR, client_pid);
					printf("Receiving... \n");
					break;  // Successfully registered a client!
				}
			}
			sleep(REGISTRATION_SLEEP);  // Still waiting...
		}
	}
	// Start Receiving
	while (ENOERR == exit_code)
	{
		exit_code = process_queue(signum, client_pid);
		if (ENODATA == exit_code)
		{
			exit_code = ENOERR;  // ENODATA is fine... there's no data yet, but not an error
		}
	}

	// DONE
	exit(exit_code);
}


void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s <SERVER_SIGNUM>\n", prog_name);
}


int process_queue(int signum, pid_t src_pid)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution
	int data = 0;         // Data read from the queue

	// PROCESS IT
	// 1. Verifies atomic variables (e.g., set, right signal number)
	result = process_queue_verify(signum);
	if (ENOERR == result)
	{
		if (skid_sig_hand_pid != src_pid)
		{
			result = ENODATA;
		}
	}

	// 2. Prints the value
	if (ENOERR == result)
	{
		data = skid_sig_hand_data_int;
		fprintf(stderr, "%c", data);
	}

	// 3. Send the read receipt
	if (ENOERR == result)
	{
		result = call_sigqueue(src_pid, signum, data);  // Send the receipt
	}

	// 4. Resets atomic variables
	if (ENOERR == result)
	{
		process_queue_reset_flags();
	}

	// DONE
	return result;
}


int process_queue_read(void)
{
	// LOCAL VARIABLE
	int result = ENOERR;                // Results of execution
	int data = skid_sig_hand_data_int;  // Integer data from the queue
	int pid = skid_sig_hand_pid;        // PID of the sending process
	int code = skid_sig_hand_sigcode;   // Signal code
	int signal = skid_sig_hand_signum;  // Signal number
	int uid = skid_sig_hand_uid;        // Real UID of the sending process
	char *sig_code_str = NULL;          // Human-readable description of the signal code

	// PROCESS IT
	// Translate the signal code
	if (ENOERR == result)
	{
		sig_code_str = translate_signal_code(signal, code, &result);
		if (result)
		{
			PRINT_ERROR(The call to translate_signal_code() failed);
			PRINT_ERRNO(result);
		}
	}
	// Print all the details
	if (ENOERR == result)
	{
		fprintf(stderr, "%s PARENT - Received value %010d inside signal number %d (%s) code "
			    "%d (%s) from UID %d at PID %d\n", DEBUG_INFO_STR, data, signal, strsignal(signal),
			    code, sig_code_str, uid, pid);
	}

	// CLEANUP
	if (NULL != sig_code_str)
	{
		free_skid_string(&sig_code_str);  // Best effort
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


int process_queue_unblock(int signum)
{
	return unblock_signal(signum, NULL);
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
