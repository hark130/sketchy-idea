/*
 *	Manually test skid_signals.h's set_signal_handler_ext() and
 *	skid_signal_handlers.h's handle_ext_read_queue_int() functions.
 *
 *	This manual test code performs the following actions:
 *	1. Install the handle_ext_read_queue_int() extended signal handler for SIGUSR1
 *	2. Fork
 *	3.a. Child: Send all values via sigqueue() and exit.
 *	3.b. Parent: Process values sent from the child process.
 *
 *	Copy/paste the following...

./code/dist/test_ssh_handle_ext_read_queue_int.bin <COUNT_LIMIT>

 *
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

/*
 *	Single point of truth for this manual test code's usage.
 */
void print_usage(const char *prog_name);

/*
 *	Call this when the signal handler indicates there's integer data to read.
 *	1. Verifies atomic variables (e.g., set, right signal number)
 *	2. Reads and prints value
 *	3. Resets atomic variables
 *	4. Unblocks the signal (to allow the handler to process the next message in the queue)
 *
 *	Returns ENOERR on success, errno on failure.
 */
int process_queue(int signum);

/*
 *	2. Reads and prints value
 */
int process_queue_read(void);

/*
 *	3. Resets atomic variables
 */
void process_queue_reset_flags(void);

/*
 *	4. Unblocks the signal (to allow the handler to process the next message in the queue)
 */
int process_queue_unblock(int signum);

/*
 *	1. Verifies atomic variables (e.g., set, right signal number)
 */
int process_queue_verify(int signum);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;       // Store errno and/or results here
	int flags = SA_RESTART;  // Modifies signal behavior: restart system calls across signals
	int signum = SIGUSR1;    // Signal number to use for communication
	int num_vals = 0;        // Number of values to be sent by the child
	pid_t my_pid = 0;        // My PID
	pid_t wait_ret = 0;      // Return value from the call to waitpid()
	int child_status = 0;    // Status information about the child process

	// INPUT VALIDATION
	if (argc != 2)
	{
		print_usage(argv[0]);
		exit_code = EINVAL;
	}
	else
	{
		num_vals = atoi(argv[1]);
		if (num_vals < 1)
		{
			print_usage(argv[0]);
			exit_code = EIO;
		}
	}

	// RUN IT
	// Block a signal
	if (!exit_code)
	{
		fprintf(stdout, "%s Blocking signal %d: %s\n", DEBUG_INFO_STR, signum, strsignal(signum));
		exit_code = block_signal(signum, NULL);
		if (exit_code)
		{
			PRINT_ERROR(The call to block_signal() failed);
			PRINT_ERRNO(exit_code);
		}
		else
		{
			printf("%s %s now temporarily blocking signal %d.\n", DEBUG_INFO_STR, argv[0], signum);
		}
	}
	// 1. Setup Signal Handler
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

	// 2. Fork
	if (ENOERR == exit_code)
	{
		my_pid = fork();
		if (-1 == my_pid)
		{
			exit_code = errno;
			if (ENOERR == exit_code)
			{
				exit_code = EINTR;  // Just in case
			}
			PRINT_ERROR(The call to fork() failed);
			PRINT_ERRNO(exit_code);
		}
	}

	// 3. Execute
	if (ENOERR == exit_code)
	{
		// 3.a. Child
		if (0 == my_pid)
		{
			for (int i = 1; i <= num_vals; i++)
			{
				sleep(1);  // Give the parent a second to reblock the signal
				exit_code = call_sigqueue(getppid(), signum, i);
				if (ENOERR != exit_code)
				{
					PRINT_ERROR(CHILD - The call_sigqueue() function failed);
					PRINT_ERRNO(exit_code);
					break;  // Stop at the first error
				}
			}
		}
		// 3.b. Parent
		else if (my_pid > 0)
		{
			exit_code = unblock_signal(signum, NULL);  // Start receiving data
			while (ENOERR == exit_code)
			{
				// Check the child process
				wait_ret = waitpid(my_pid, &child_status, WNOHANG);  // Is the child alive?
				if (my_pid == wait_ret)
				{
					// The child's state has changed
					if (WIFEXITED(child_status) || WIFSIGNALED(child_status) \
						|| WIFSTOPPED(child_status))
					{
						// The child is no more
						if (num_vals > 0)
						{
							PRINT_WARNG(The child has exited without sending all expected values);
						}
						else
						{
							printf("%s PARENT - The child completed all values and exited\n",
						           DEBUG_INFO_STR);
						}
						break;
					}
				}
				else if (0 == wait_ret)
				{
					// The child didn't change state yet... be patient
				}
				else if (-1 == wait_ret)
				{
					PRINT_ERROR(PARENT - The waitpid() call failed);
					exit_code = child_status;
					PRINT_ERRNO(exit_code);
					break;
				}
				else
				{
					PRINT_ERROR(PARENT - The call to waitpid() reported an unknown PID);
					FPRINTF_ERR("PARENT - The call to waitpid() reported an unknown PID: %d\n",
						        wait_ret);
					break;
				}

				// Process Data
				exit_code = process_queue(signum);
				if (ENOERR == exit_code)
				{
					num_vals--;  // We got one
				}
				else if (ENODATA == exit_code)
				{
					exit_code = ENOERR;  // Ignore a lack of data
				}
				else
				{
					break;  // This error is bad so stop looping
				}
			}
		}
	}

	// DONE
	exit(exit_code);
}


void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s <COUNT_LIMIT>\n", prog_name);
}


int process_queue(int signum)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// PROCESS IT
	// 1. Verifies atomic variables (e.g., set, right signal number)
	result = process_queue_verify(signum);

	// 2. Reads and prints value
	if (ENOERR == result)
	{
		result = process_queue_read();
	}

	// 3. Resets atomic variables
	if (ENOERR == result)
	{
		process_queue_reset_flags();
	}

	// 4. Unblocks the signal (to allow the handler to process the next message in the queue)
	if (ENOERR == result)
	{
		result = process_queue_unblock(signum);
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
